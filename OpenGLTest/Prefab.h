#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class Prefab : public EditorObject
{
public:
	Prefab();
	~Prefab();

    Sprite* editorSprite = nullptr;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;
};
