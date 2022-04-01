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

vec2 EditorObject::DrawProperties(vec2 startPos)
{
	vec2 drawPos = startPos;
	drawPos.y -= Editor::DrawProperty(drawPos, "Name :", name, Editor::panelSize - 20).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Position :", position, Editor::panelSize - 20).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Parallax :", std::to_string(parallax), Editor::panelSize - 20).y;

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
