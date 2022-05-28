#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

constexpr int CHECKPOINT_EVENT_COUNT = 1;

using namespace glm;

class Checkpoint : public EditorObject
{
public:
	Checkpoint();
	~Checkpoint();

    Sprite* editorSprite = nullptr;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	void SaveGame();

private:
	static ObjectEvent events[CHECKPOINT_EVENT_COUNT];
};

