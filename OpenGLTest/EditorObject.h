#pragma once

#include "Collider.h"
#include "Editor.h"

#include <glm/common.hpp>
#include <string>

using namespace glm;

class EditorSaveManager;
class EditorObject
{
public:
	EditorObject(vec3 position);
	virtual ~EditorObject();

	std::string typeName;

	int ID;
	std::string name;

	vec3 GetEditPos();
	float GetEditRotation();
	vec2 GetEditScale();

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

	/// <summary>
	/// Save the object, please use EditorSaveManager api
	/// because of inhiterance, DO NOT INCLUDE StartObject() AND EndObject()
	/// </summary>
	virtual void Save();
	virtual void Load(std::map<std::string, std::string>* props);

	/// <summary>
	/// Restores the sprites as it was before being disabled
	/// </summary>
	virtual void Enable();
	/// <summary>
	/// The object will not be visible, clickable or everything else. DON'T FORGET TO CALL BASE FUNCTION WHEN OVERRIDING
	/// </summary>
	virtual void Disable();
	bool IsEnabled();
	/// <summary>
	/// TOGGLE TOGGLE TOGGLE!
	/// </summary>
	void ToggleEnabled();

protected:
	bool enabled;
	vec3 editorPosition;
	float editorRotation;
	vec2 editorSize;
};

