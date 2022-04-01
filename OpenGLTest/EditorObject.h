#pragma once

#include "Editor.h"
#include "Collider.h"

#include <glm/common.hpp>
#include <string>

using namespace glm;

class EditorObject
{
public:
	EditorObject(vec2 position);
	~EditorObject();

	int ID;
	std::string name;

	vec2 GetPos();
	virtual vec2 SetPos(vec2 pos);
	float parallax;

	Collider* clickCollider;

	virtual std::string Save();
	static EditorObject* LoadBaseData(std::string data);

	virtual vec2 DrawProperties(vec2 drawPos);

private:
	vec2 position;
};

