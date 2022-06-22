#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class TextRenderer : public EditorObject
{
public:
	TextRenderer();
	~TextRenderer();

    Sprite* editorSprite = nullptr;

	std::string content;
	float fontSize = 1;
	vec4 color = vec4(1);

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;
};

