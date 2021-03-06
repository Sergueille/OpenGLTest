#include "Prefab.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "PrefabRelay.h"

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

	if (prefabRelay == nullptr)
	{
		eventCount = 0;
	}
	else
	{
		eventCount = (int)prefabRelay->eventLists.size();

		if (eventCount > 0)
		{
			for (int i = 0; i < eventCount; i++)
			{
				events[i] = ObjectEvent{
					prefabRelay->eventNames[i],
					[i](EditorObject* object, void* param) ->void { ((Prefab*)object)->prefabRelay->CallEventList(i); },
				};
			}
		}
	}
}

vec2 Prefab::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y + Editor::margin;

	bool pressed;
	drawPos.y -= Editor::UIButton(drawPos, "Reload", &pressed).y + Editor::margin;
	if (pressed) ReloadPrefab();

	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &editorRotation, Editor::panelPropertiesX, strID + "ori").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Scale", &editorSize, Editor::panelPropertiesX, strID + "scale").y;

	std::string editPathName = GetPath();
	drawPos.y -= Editor::FileSelector(drawPos, "Map path", &editPathName, &Editor::mapFiles, Editor::panelPropertiesX, strID + "path").y;
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
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->prefabObjects = std::list<EditorObject*>();
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
	if (prefabObjects.size() == 0)
	{
		*minRes = GetEditPos();
		*maxRes = GetEditPos();
		return;
	}

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

void Prefab::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*resCount = eventCount;
	*res = &events[0];
}

void Prefab::ResetIngameState()
{
	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		(*it)->ResetIngameState();
	}
}

void Prefab::OnMainLoop()
{
	EditorObject::OnMainLoop();

	if (!enabled) return;

	if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos() + vec3(0, 0, 50);
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		(*it)->OnMainLoop();
	}
}
