#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "TweenManager.h"

using namespace glm;

constexpr int TRANS_MODIFIER_EVENT_COUNT = 5;

class TransformModifier : public EditorObject
{
public:
	TransformModifier();
	~TransformModifier();

    Sprite* editorSprite = nullptr;

	int targetID = -1;
	bool relative = true;
	vec2 targetPos = vec2(0);
	float targetRotation = 0;
	vec2 targetSize = vec2(1);

	float duration = 1;
	EaseType easeType = linear;

	EventList onFinished;

	static ObjectEvent events[TRANS_MODIFIER_EVENT_COUNT];

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** firstEvent, int* count) override;

	virtual void ResetIngameState() override;

	void SetPosition();
	void SetRotation();
	void SetScale();
	void SetAllTransforms();

	void CancelAll();

private:
	EditorObject* targetObject;

	TweenAction<vec2>* moveAction = nullptr;
	TweenAction<float>* roateAction = nullptr;
	TweenAction<vec2>* scaleAction = nullptr;

	vec3 targetStartPos;
	float targetStartRotation;
	vec2 targetStartScale;
};
