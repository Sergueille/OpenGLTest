#include "PrefabRelay.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

PrefabRelay::PrefabRelay() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\prefabRelay.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "PrefabRelay";

	eventLists = std::vector<EventList>();
	eventNames = std::vector<std::string>();
}

PrefabRelay::~PrefabRelay()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
}

vec2 PrefabRelay::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);
	bool pressed;

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	for (int i = 0; i < (int)eventLists.size(); i++)
	{
		std::string iStr = std::to_string(i);
		drawPos.y -= Editor::margin;
		drawPos.y -= Editor::DrawProperty(drawPos, "Event name", &eventNames[i], Editor::panelPropertiesX, strID + "event name" + iStr).y - Editor::margin;
		drawPos.y -= eventLists[i].DrawInPanel(drawPos, eventNames[i]).y;
		drawPos.y -= Editor::UIButton(drawPos, "Remove event list", &pressed).y;

		if (pressed)
		{
			eventLists.erase(eventLists.begin() + i);
			break;
		}
	}

	drawPos.y -= Editor::margin;
	drawPos.y -= Editor::UIButton(drawPos, "Add event list", &pressed).y;

	if (pressed)
	{
		eventLists.push_back(EventList());
		eventNames.push_back("Event list with no name");
	}

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* PrefabRelay::Copy()
{
	PrefabRelay* newObj = new PrefabRelay(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->position, oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void PrefabRelay::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	int eventListsCount = 0;
	EditorSaveManager::IntProp(props, "eventListsCount", &eventListsCount);

	eventLists.clear();

	for (int i = 0; i < eventListsCount; i++)
	{
		std::string propName = "eventList" + std::to_string(i);
		eventLists.push_back(EventList());
		EventList::Load(&eventLists[i], (*props)[propName]);

		eventNames.push_back((*props)[propName + "name"]);
	}
}

void PrefabRelay::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("eventListsCount", (int)eventLists.size());
	for (int i = 0; i < (int)eventLists.size(); i++)
	{
		std::string propName = "eventList" + std::to_string(i);
		EditorSaveManager::WriteProp(propName, eventLists[i].GetString());

		EditorSaveManager::WriteProp(propName + "name", eventNames[i]);
	}
}

void PrefabRelay::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void PrefabRelay::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void PrefabRelay::CallEventList(int i)
{
	eventLists[i].Call(this);
}

void PrefabRelay::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}
