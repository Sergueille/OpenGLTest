#include "EditorSprite.h"

EditorSprite::EditorSprite(glm::vec3 position, glm::vec2 size, float rotate) : Sprite(nullptr, position, size, rotate), EditorObject(position)
{
	clickCollider = new RectCollider(position, size, rotate, false);

	this->editorRotation = rotate;
	this->editorSize = size;

	typeName = "EditorSprite";

	DrawOnMainLoop();
}

vec2 EditorSprite::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec3 startPos = drawPos;

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Size", &editorSize, Editor::panelPropertiesX, strID + "size").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &editorRotation, Editor::panelPropertiesX, strID + "rotate").y;
	UpdateTransform();
	drawPos.y -= Editor::DrawProperty(drawPos, "Color", &color, Editor::panelPropertiesX, strID + "color", true).y;

	std::string noTextureDisplay = "No texture";
	std::string texName = texture == nullptr? noTextureDisplay : texture->path;
	drawPos.y -= Editor::DrawProperty(drawPos, "Texture", &texName, Editor::panelPropertiesX, strID + "texture").y;

	if (texName == "" || texName == noTextureDisplay)
	{
		texture = nullptr;
	}
	else
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

EditorObject* EditorSprite::Copy()
{
	EditorSprite* newObj = new EditorSprite(*this);

	// copy collider
	RectCollider* oldCollider = (RectCollider*)this->clickCollider;
	newObj->clickCollider = new RectCollider(oldCollider->position, oldCollider->size, oldCollider->orientation, false);

	// subscribe again to main loop
	newObj->isDrawnOnMainLoop = false;
	newObj->DrawOnMainLoop();

	// add to editor list because it's not called automatically because no constructor were used to create object
	Editor::editorObjects.push_back(newObj);

	return newObj;
}

void EditorSprite::Load(std::map<std::string, std::string>* props)
{
	editorSize = EditorSaveManager::StringToVector2((*props)["size"]);
	editorRotation = std::stof((*props)["rotation"]);
	color = EditorSaveManager::StringToVector4((*props)["color"]);

	std::string textureName = (*props)["texturePath"];
	if (textureName != "")
	{
		texture = RessourceManager::GetTexture(textureName);
	}

	EditorObject::Load(props);
}

void EditorSprite::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("size", editorSize);
	EditorSaveManager::WriteProp("rotation", std::to_string(editorRotation));
	EditorSaveManager::WriteProp("color", color);
	if (texture != nullptr)
		EditorSaveManager::WriteProp("texturePath", texture->path);
}

void EditorSprite::UpdateTransform()
{
	this->position = editorPosition;
	this->rotate = editorRotation;
	this->size = editorSize;

	clickCollider->position = position;
	((RectCollider*)clickCollider)->orientation = rotate;
	((RectCollider*)clickCollider)->size = size;
}
