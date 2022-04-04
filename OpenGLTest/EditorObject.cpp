#include "EditorObject.h"

#include <string>

using namespace glm;

EditorObject::EditorObject(vec3 position)
{
	Editor::editorObjects.push_back(this);
	this->SetEditPos(position);

	Editor::IDmax++;
	this->ID = Editor::IDmax;
}

EditorObject::~EditorObject()
{
	Editor::editorObjects.remove(this);
	if (clickCollider) delete clickCollider;
}

vec3 EditorObject::GetEditPos()
{
	return editorPosition;
}

vec3 EditorObject::SetEditPos(vec3 pos)
{
	editorPosition = pos;
	if (clickCollider) 
		clickCollider->position = editorPosition;
	return editorPosition;
}

vec2 EditorObject::DrawProperties(vec3 startPos)
{
	std::string strID = std::to_string(ID);

	vec3 drawPos = startPos;
	drawPos.y -= Editor::DrawProperty(drawPos, "Name", &name, Editor::panelPropertiesX, strID + "name").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Position", &editorPosition, Editor::panelPropertiesX, strID + "pos").y;
	SetEditPos(editorPosition);

	vec2 res = vec2(drawPos - startPos);
	res.y *= -1;
	return res;
}

std::string EditorObject::Save()
{
	return "";
}

EditorObject* EditorObject::LoadBaseData(std::string data)
{
	return NULL;
}
