#pragma once

#include "Sprite.h"
#include "EditorObject.h"
#include "RectCollider.h"
#include "RessourceManager.h"

using namespace glm;

class EditorSprite : Sprite, EditorObject
{
public:
	EditorSprite(glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), float rotate = 0.0f);

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;
};
