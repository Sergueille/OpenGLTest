#include "Prefab.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

Prefab::Prefab() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("engine\\prefab.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }

	prefabObjects = std::list<EditorObject*>();
            
	typeName = "Prefab";
}

Prefab::~Prefab()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		delete (*it);
		*it = nullptr;
	}
}

std::string Prefab::GetPath()
{
	return prefabPath;
}

void Prefab::SetPath(std::string newName)
{
	if (newName == prefabPath) return;

	prefabPath = newName;
	ReloadPrefab();
}

void Prefab::ReloadPrefab()
{
	// Delete old objects
	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		delete (*it);
		*it = nullptr;
	}
	prefabObjects = std::list<EditorObject*>();

	// Load file
	EditorSaveManager::LoadPrefab(this);
}

vec2 Prefab::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &editorRotation, Editor::panelPropertiesX, strID + "ori").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Scale", &editorSize, Editor::panelPropertiesX, strID + "scale").y;

	std::string editPathName = GetPath();
	drawPos.y -= Editor::DrawProperty(drawPos, "Map path", &editPathName, Editor::panelPropertiesX, strID + "path").y;
	SetPath(editPathName);

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

vec2 Prefab::DrawActions(vec3 drawPos)
{
	vec2 startPos = drawPos;
	vec2 baseSize = EditorObject::DrawActions(drawPos);
	drawPos.x += baseSize.x + Editor::margin;

	if (GetPath() != "")
	{
		bool pressed;
		drawPos.x += Editor::UIButton(drawPos, "Open file", &pressed).x;

		if (pressed)
		{
			EditorSaveManager::LoadLevel(GetPath(), true);
		}
	}

	drawPos.y += baseSize.y;
	return Abs(startPos - vec2(drawPos));
}

EditorObject* Prefab::Copy()
{
	Prefab* newObj = new Prefab(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->position, oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	newObj->prefabObjects = std::list<EditorObject*>();
	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		newObj->prefabObjects.push_back((*it)->Copy());

		if (newObj->prefabObjects.back()->parentID == this->ID)
		{
			newObj->prefabObjects.back()->SetParent(newObj);
		}
	}

	return newObj;
}

void Prefab::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);
	prefabPath = (*props)["path"];

	EditorSaveManager::FloatProp(props, "ori", &editorRotation);
	editorSize = EditorSaveManager::StringToVector2((*props)["scale"], vec2(1));
}

void Prefab::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("path", GetPath());
	EditorSaveManager::WriteProp("ori", editorRotation);
	EditorSaveManager::WriteProp("scale", editorSize);
}

void Prefab::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();

	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		(*it)->Enable();
	}
}

void Prefab::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();

	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		(*it)->Disable();
	}
}

void Prefab::GetAABB(vec2* minRes, vec2* maxRes)
{
	prefabObjects.front()->GetAABB(minRes, maxRes);

	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		vec2 objMin, objMax;
		(*it)->GetAABB(&objMin, &objMax);

		if (objMin.x < minRes->x)
			minRes->x = objMin.x;
		if (objMin.y < minRes->y)
			minRes->y = objMin.y;
		if (objMax.x > maxRes->x)
			maxRes->x = objMax.x;
		if (objMax.y > minRes->y)
			maxRes->y = objMax.y;
	}
}

void Prefab::UpdateTransform()
{
	EditorObject::UpdateTransform();

	if (editorSprite != nullptr)
	{
		editorSprite->position = editorPosition + vec3(0, 0, 50);
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}
