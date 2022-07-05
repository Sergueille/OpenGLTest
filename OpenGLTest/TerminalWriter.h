#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

constexpr int TERM_WRIT_EVENT_COUNT = 2;
constexpr float timeBetweenLines = 2.0f;

class TerminalWriter : public EditorObject
{
public:
	TerminalWriter();
	~TerminalWriter();

	static ObjectEvent events[TERM_WRIT_EVENT_COUNT];

	std::vector<std::string> keys;
	EventList onFinished;

	bool writeRandom = false; // Write a random line instead of all

    Sprite* editorSprite = nullptr;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	void Write();

private:
	float nextWriteTime = 0;
	bool shouldWrite = false;
	int writePos = 0;
};

