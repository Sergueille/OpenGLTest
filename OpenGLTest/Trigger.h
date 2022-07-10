#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

constexpr int TRIGGER_EVENT_COUNT = 2;

class Trigger : public EditorObject
{
public:
	Trigger();
	~Trigger();

	static ObjectEvent events[TRIGGER_EVENT_COUNT];

	EventList onEnter;
	EventList onExit;

	bool once = false;
	bool needPlayerInteraction = false;

	bool killPlayer = false;

    Sprite* editorSprite = nullptr;

	bool collideWithPlayer = false;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	virtual void ResetIngameState() override;

private: 
	bool hasAlredyTriggered = false;
};

