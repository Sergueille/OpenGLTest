#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

constexpr int LEVEL_END_EVENT_COUNT = 1;

class LevelEnd : public EditorObject
{
public:
	LevelEnd();
	~LevelEnd();

    Sprite* editorSprite = nullptr;

	std::string nextLevel;
	bool returnToMenu = false;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	void EndLevel();

private:
	static const ObjectEvent events[LEVEL_END_EVENT_COUNT];

	bool isEndingLevel = false;
};

