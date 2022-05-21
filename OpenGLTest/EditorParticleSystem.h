#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "ParticleSystem.h"

using namespace glm;

class EditorParticleSystem : public EditorObject, public ParticleSystem
{
public:
	EditorParticleSystem();
	~EditorParticleSystem();

    Sprite* editorSprite = nullptr;
    Sprite* previewSprite = nullptr;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void OnSelected() override;
	virtual void OnUnselected() override;
};

