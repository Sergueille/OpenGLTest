#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include <soloud.h>

using namespace glm;

constexpr int SOUND_POINT_EVENT_COUNT = 2;

class SoundPoint : public EditorObject
{
public:
	SoundPoint();
	~SoundPoint();

    Sprite* editorSprite = nullptr;

	static ObjectEvent events[SOUND_POINT_EVENT_COUNT];

	std::string soundName;
	float volume = 1;

	bool spatial = true;
	float maxDist = 20;
	float minDist = 1;

	bool autoStart = true;
	bool loop = true;

	bool isPlaying = false;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	void PlaySound();
	void StopSound();

private:
	SoLoud::handle handle = -1;

	bool playingBeforeDisabled = false;
};

