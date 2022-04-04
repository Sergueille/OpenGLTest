#include "EditorSprite.h"

EditorSprite::EditorSprite(glm::vec3 position, glm::vec2 size, float rotate) : Sprite(nullptr, position, size, rotate), EditorObject(position)
{
	clickCollider = new RectCollider(position, size, rotate);
}

vec3 EditorSprite::SetEditPos(vec3 pos)
{
	this->position = pos;
	return EditorObject::SetEditPos(pos);
}

vec2 EditorSprite::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec3 startPos = drawPos;

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Size", &size, Editor::panelPropertiesX, strID + "size").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &rotate, Editor::panelPropertiesX, strID + "rotate").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Color", &color, Editor::panelPropertiesX, strID + "color", true).y;
	UpdateTransform();

	std::string noTextureDisplay = "No texture";
	std::string texName = texture == nullptr? noTextureDisplay : texture->path;
	drawPos.y -= Editor::DrawProperty(drawPos, "Texture", &texName, Editor::panelPropertiesX, strID + "texture").y;

	if (texName != "")
		texture = nullptr;

	if (texName != noTextureDisplay)
	{
		if (texture == nullptr || texName != texture->path)
		{
			texture = RessourceManager::GetTexture(texName);
		}
	}

	vec2 res = drawPos - startPos;
	res.y *= -1;
	return res;
}

void EditorSprite::UpdateTransform()
{
	clickCollider->position = position;
	((RectCollider*)clickCollider)->orientation = rotate;
	((RectCollider*)clickCollider)->size = size;
}
