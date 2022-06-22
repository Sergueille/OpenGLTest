#include "PrefabCloner.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

PrefabCloner::PrefabCloner() : Prefab()
{
	if (editorSprite != nullptr)
		editorSprite->texture = RessourceManager::GetTexture("Engine\\prefabCloner.png");
	typeName = "PrefabCloner";

	lastTime = Utility::time;
}

PrefabCloner::~PrefabCloner()
{
	if (!Editor::enabled)
	{
		for (auto it = clonedObjects.begin(); it != clonedObjects.end(); it++)
		{
			delete (*it);
		}
		clonedObjects.clear();
	}
}

void PrefabCloner::ReloadPrefab()
{
	Prefab::ReloadPrefab();
	if (!Editor::enabled)
	{
		// Disable all prefab objects, they will be used as a template
		for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
		{
			(*it)->Disable();
		}
	}
}

vec2 PrefabCloner::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= Prefab::DrawProperties(drawPos).y;

	drawPos.y -= Editor::CheckBox(drawPos, "Clone with timer", &cloneOnTimer, Editor::panelPropertiesX).y;
	if (cloneOnTimer)
	{
		drawPos.y -= Editor::DrawProperty(drawPos, "Delay", &delay, Editor::panelPropertiesX, strID + "delay").y;
	}
	drawPos.y -= Editor::margin;

	drawPos.y -= Editor::CheckBox(drawPos, "Delete after timer", &deleteAfterTime, Editor::panelPropertiesX).y;
	if (deleteAfterTime)
	{
		drawPos.y -= Editor::DrawProperty(drawPos, "Delete time", &deleteTime, Editor::panelPropertiesX, strID + "deltime").y;
	}
	drawPos.y -= Editor::margin;

	drawPos.y -= Editor::DrawProperty(drawPos, "Max instances count", &maxInstances, Editor::panelPropertiesX, strID + "maxCount").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

void PrefabCloner::Load(std::map<std::string, std::string>* props)
{
	Prefab::Load(props);

	cloneOnTimer = (*props)["cloneOnTimer"] == "1";
	EditorSaveManager::FloatProp(props, "delay", &delay);

	deleteAfterTime = (*props)["deleteAfterTime"] == "1";
	EditorSaveManager::FloatProp(props, "deleteTime", &deleteTime);

	EditorSaveManager::IntProp(props, "maxInstances", &maxInstances);
}

void PrefabCloner::Save()
{
	Prefab::Save();

	EditorSaveManager::WriteProp("cloneOnTimer", cloneOnTimer);
	EditorSaveManager::WriteProp("delay", delay);
	EditorSaveManager::WriteProp("deleteAfterTime", deleteAfterTime);
	EditorSaveManager::WriteProp("deleteTime", deleteTime);
	EditorSaveManager::WriteProp("maxInstances", maxInstances);
}

void PrefabCloner::Enable()
{
	Prefab::Enable();
}

void PrefabCloner::Disable()
{
	Prefab::Disable();
}

void PrefabCloner::CreateClone()
{
	if (Editor::enabled)
	{
		std::cerr << "CreateClone() called on a PrefabCloner in editor, that shouldn't happen" << std::endl;
		return;
	} 

	// Create a new clone from template
	PrefabClone* clone = new PrefabClone();
	clone->startTime = Utility::time;

	clone->objects = new std::list<EditorObject*>();
	for (auto it = prefabObjects.begin(); it != prefabObjects.end(); it++)
	{
		(*it)->Enable();
		EditorObject* copy = (*it)->Copy();
		clone->objects->push_back(copy);
		copy->contextList = clone->objects;
		(*it)->Disable();
	}

	clonedObjects.push_back(clone);

	// Delete last if too many objects
	if ((int)clonedObjects.size() > maxInstances)
	{
		delete clonedObjects.front();
		clonedObjects.pop_front();
	}
}

void PrefabCloner::OnMainLoop()
{
	Prefab::OnMainLoop();
	if (!enabled) return;

	for (auto clone = clonedObjects.begin(); clone != clonedObjects.end(); clone++)
	{
		for (auto obj = (*clone)->objects->begin(); obj != (*clone)->objects->end(); obj++)
		{
			(*obj)->OnMainLoop();
		}
	}

	if (!Editor::enabled)
	{
		if (cloneOnTimer)
		{
			if (Utility::time > lastTime + delay)
			{
				lastTime = lastTime + delay;
				EventManager::DoInOneFrame([this] { CreateClone(); });
			}
		}

		if (deleteAfterTime)
		{
			for (auto it = clonedObjects.begin(); it != clonedObjects.end(); it++)
			{
				if (Utility::time > (*it)->startTime + deleteTime)
				{
					delete (*it);
					it = clonedObjects.erase(it);
				}
			}
		}
	}
}

PrefabClone::~PrefabClone()
{
	for (auto it = objects->begin(); it != objects->end(); it++)
	{
		delete (*it);
	}
	objects->clear();
	delete objects;
}
