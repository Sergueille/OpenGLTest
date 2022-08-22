#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include <soloud.h>

using namespace glm;

class SoundArea : public EditorObject
{
public:
	SoundArea();
	~SoundArea();

    Sprite* editorSprite = nullptr;

	std::string soundName;
	float volume = 1;
	float fadeIn = 1;
	float fadeOut = 1;
	bool isMusic = false;
	int syncAreaID = -1;

	bool isPlaying = false;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	void PlaySound();
	void StopSound();

private:
	static SoundArea* mainArea;
	static SoundArea* mainMusicArea;

	SoLoud::handle handle;
	float relativeVolume = 0;
	EditorObject* syncArea = nullptr;
};

