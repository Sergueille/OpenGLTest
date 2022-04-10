#pragma once

#include "Collider.h"
#include "Editor.h"

#include <glm/common.hpp>
#include <string>

using namespace glm;

class EditorObject
{
public:
	EditorObject(vec3 position);
	virtual ~EditorObject();

	int ID;
	std::string name;

	vec3 GetEditPos();
	vec3 SetEditPos(vec3 pos);
	float SetEditRotation(float rot);
	vec2 SetEditScale(vec2 scale);

	/// <summary>
	/// Call this to replace collider, sprites, etc.
	/// </summary>
	virtual void UpdateTransform();

	float parallax;

	/// <summary>
	/// Collider used to click on object in the editor
	/// </summary>
	Collider* clickCollider;

	/// <summary>
	/// Draws properties of object in the editor panel
	/// </summary>
	virtual vec2 DrawProperties(vec3 drawPos);
	/// <summary>
	/// Draws buttons under the properties
	/// </summary>
	virtual vec2 DrawActions(vec3 drawPos);

	virtual EditorObject* Copy() = 0;

	// DO NOT SET MANUALLY, use setEditPos, setEditRotation...
	vec3 editorPosition;
	float editorRotation;
	vec2 editorSize;
};

