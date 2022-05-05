#pragma once
#include "EditorObject.h"
class Light : public EditorObject
{
public:
	Light();
	~Light();

	Sprite* editorSprite = nullptr;

	float size = 5;
	vec4 color = vec4(1);
	float intensity = 1;
	float innerAngle = 360;
	float outerAngle = 360;

	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual vec2 DrawProperties(vec3 drawPos) override;

	virtual void Save() override;
	virtual void Load(std::map<std::string, std::string>* props) override;
};

