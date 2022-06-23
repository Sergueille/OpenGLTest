#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include <soloud.h>

using namespace glm;

class SoundPoint : public EditorObject
{
public:
	SoundPoint();
	~SoundPoint();

    Sprite* editorSprite = nullptr;

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

	void PlaySound();
	void StopSound();

private:
	SoLoud::handle handle = -1;
};

