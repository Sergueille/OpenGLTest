#include "EditorObject.h"

#include <string>

using namespace glm;

EditorObject::EditorObject(vec2 position)
{
	Editor::editorObjects.push_back(this);
	this->SetPos(position);
}

EditorObject::~EditorObject()
{
	Editor::editorObjects.remove(this);
	if (clickCollider) delete clickCollider;
}

vec2 EditorObject::GetPos()
{
	return position;
}

vec2 EditorObject::SetPos(vec2 pos)
{
	position = pos;
	if (clickCollider) 
		clickCollider->position = position;
	return position;
}

vec2 EditorObject::DrawProperties(vec3 startPos)
{
	std::string strID = std::to_string(ID);
	float propX = Editor::panelSize / 2.0f;

	vec3 drawPos = startPos;
	drawPos.y -= Editor::DrawProperty(drawPos, "Name", &name, propX, strID + "name").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Position", &position, propX, strID + "pos").y;
	SetPos(position);

	return drawPos - startPos;
}

std::string EditorObject::Save()
{
	return "";
}

EditorObject* EditorObject::LoadBaseData(std::string data)
{
	return NULL;
}
