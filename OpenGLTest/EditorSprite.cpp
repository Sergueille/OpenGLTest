#include "EditorSprite.h"

EditorSprite::EditorSprite(glm::vec3 position, glm::vec2 size, float rotate) : Sprite(nullptr, position, size, rotate), EditorObject(position)
{
	clickCollider = new RectCollider(position, size, rotate, false);

	SetEditRotation(rotate);
	SetEditScale(size);

	typeName = "EditorSprite";

	DrawOnMainLoop();
}

EditorSprite::~EditorSprite()
{
	delete clickCollider;
	clickCollider = nullptr;
}

vec2 EditorSprite::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Size", &editorSize, Editor::panelPropertiesX, strID + "size").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &editorRotation, Editor::panelPropertiesX, strID + "rotate").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Color", &color, Editor::panelPropertiesX, strID + "color", true).y;

	bool collide = clickCollider->MustCollideWithPhys();
	drawPos.y -= Editor::CheckBox(drawPos, "Collide with physics", &collide, Editor::panelPropertiesX).y;
	clickCollider->SetCollideWithPhys(collide);

	drawPos.y -= Editor::DrawProperty(drawPos, "Texture test", &texture, Editor::panelPropertiesX, strID + "texture").y;

	drawPos.y -= Editor::DrawProperty(drawPos, "UV start", &UVStart, Editor::panelPropertiesX, strID + "UVstart").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "UV end", &UVEnd, Editor::panelPropertiesX, strID + "UVend").y;

	drawPos.y -= Editor::CheckBox(drawPos, "Is lit", &isLit, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* EditorSprite::Copy()
{
	EditorSprite* newObj = new EditorSprite(*this);

	// copy collider
	RectCollider* oldCollider = (RectCollider*)this->clickCollider;
	newObj->clickCollider = new RectCollider(oldCollider->position, oldCollider->size, oldCollider->orientation, oldCollider->MustCollideWithPhys());

	// subscribe again to main loop
	newObj->isDrawnOnMainLoop = false;
	newObj->DrawOnMainLoop();
	newObj->SubscribeToEditorObjectFuncs();

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

	bool collide = (*props)["collideWithPhys"] == "1";
	clickCollider->SetCollideWithPhys(collide);

	UVStart = EditorSaveManager::StringToVector2((*props)["UVstart"], UVStart);
	UVEnd = EditorSaveManager::StringToVector2((*props)["UVend"], UVEnd);

	isLit = (*props)["isLit"] == "1";

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
	EditorSaveManager::WriteProp("collideWithPhys", clickCollider->MustCollideWithPhys());
	EditorSaveManager::WriteProp("UVstart", UVStart);
	EditorSaveManager::WriteProp("UVend", UVEnd);
	EditorSaveManager::WriteProp("isLit", isLit);
}

void EditorSprite::Enable()
{
	EditorObject::Enable();
	DrawOnMainLoop();
	clickCollider->enabled = true;
}

void EditorSprite::Disable()
{
	EditorObject::Disable();
	StopDrawing();
	clickCollider->enabled = false;
}

void EditorSprite::UpdateTransform()
{
	EditorObject::UpdateTransform();
	this->position = GetEditPos();
	this->rotate = GetEditRotation();
	this->size = GetEditScale();

	clickCollider->position = position;
	((RectCollider*)clickCollider)->orientation = rotate;

	if (texture != nullptr)
	{
		((RectCollider*)clickCollider)->size = size * vec2(texture->ratio, 1);
	}
	else
	{
		((RectCollider*)clickCollider)->size = size;
	}
}
