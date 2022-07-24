#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "TweenManager.h"
#include "TWeenAction.h"

using namespace glm;

constexpr int EDITOR_SPRITE_EVENT_COUNT = 3;

class EditorSprite : Sprite, public EditorObject
{
public:
	EditorSprite(glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), float rotate = 0.0f);
	~EditorSprite();

	static ObjectEvent events[EDITOR_SPRITE_EVENT_COUNT];

	bool visibleOnlyInEditor = false;
	float fadeDuration = 1;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;

	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	virtual void ResetIngameState() override;

	virtual void Fade(float duration);

private:
	TweenAction<float>* fadeAction = nullptr;
};
