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
	~EditorObject();

	int ID;
	std::string name;

	vec3 GetEditPos();
	virtual vec3 SetEditPos(vec3 pos);
	float parallax;

	Collider* clickCollider;

	virtual std::string Save();
	static EditorObject* LoadBaseData(std::string data);

	virtual vec2 DrawProperties(vec3 drawPos);

private:
	vec3 editorPosition;
};

