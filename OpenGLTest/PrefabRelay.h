#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class PrefabRelay : public EditorObject
{
public:
	PrefabRelay();
	~PrefabRelay();

    Sprite* editorSprite = nullptr;

	std::vector<EventList> eventLists;
	std::vector<std::string> eventNames;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	void CallEventList(int i);
};

