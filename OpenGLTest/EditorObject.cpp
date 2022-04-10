#include "EditorObject.h"

#include <string>

using namespace glm;

EditorObject::EditorObject(vec3 position)
{
	this->SetEditPos(position);

	Editor::IDmax++;
	this->ID = Editor::IDmax;
}

EditorObject::~EditorObject()
{
	if (clickCollider) delete clickCollider;
}

vec3 EditorObject::GetEditPos()
{
	return editorPosition;
}

vec3 EditorObject::SetEditPos(vec3 pos)
{
	editorPosition = pos;
	UpdateTransform();
	return editorPosition;
}

float EditorObject::SetEditRotation(float rot)
{
	editorRotation = rot;
	UpdateTransform();
	return rot;
}

vec2 EditorObject::SetEditScale(vec2 scale)
{
	editorSize = scale;
	UpdateTransform();
	return scale;
}

void EditorObject::UpdateTransform()
{
	if (clickCollider)
		clickCollider->position = editorPosition;
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

vec2 EditorObject::DrawActions(vec3 drawPos)
{
	vec3 startPos = drawPos;

	bool res;
	drawPos.x += Editor::Button(drawPos, "Destroy", &res).x + Editor::margin;

	if (res)
	{
		Editor::RemoveObject(this);
		return vec2(0);
	}

	vec2 lastBtnSize = Editor::Button(drawPos, "Duplicate", &res);

	if (res)
	{
		EditorObject* newObj = Copy();
		Editor::SelectObject(newObj);
	}

	drawPos.x += lastBtnSize.x;
	drawPos.y += lastBtnSize.y * -1;

	vec2 size = vec2(drawPos - startPos);
	size.y *= -1;
	return size;
}
