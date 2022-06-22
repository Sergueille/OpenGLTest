#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "TweenManager.h"

constexpr int LOGIC_RELAY_EVENT_COUNT = 4;

using namespace glm;

class LogicRelay : public EditorObject
{
public:
	LogicRelay();
	~LogicRelay();

	bool triggerOnStart = false;
	EventList onTrigger;
	EventList onAltTrigger;

	float delay = 0;

	bool isAlt = false;

    Sprite* editorSprite = nullptr;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	void Trigger();

private:
	static const ObjectEvent events[LOGIC_RELAY_EVENT_COUNT];

	TweenAction<float>* waitAction = nullptr;

	void TriggerNow();
};

