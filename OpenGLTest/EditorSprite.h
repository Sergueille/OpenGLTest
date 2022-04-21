#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RectCollider.h"
#include "RessourceManager.h"

using namespace glm;

class EditorSprite : Sprite, public EditorObject
{
public:
	EditorSprite(glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), float rotate = 0.0f);
	~EditorSprite();

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;

	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;
};
