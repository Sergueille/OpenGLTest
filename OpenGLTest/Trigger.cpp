#include "Trigger.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "Player.h"

ObjectEvent Trigger::events[TRIGGER_EVENT_COUNT] = {
	ObjectEvent {
		"Enable",
		[](EditorObject* object, void* param) { ((Trigger*)object)->Enable(); }
	},
	ObjectEvent {
		"Disable",
		[](EditorObject* object, void* param) { ((Trigger*)object)->Disable(); }
	},
};

Trigger::Trigger() : EditorObject(vec3(0))
{
	clickCollider = new RectCollider(vec2(0), vec2(1), 0, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(nullptr, vec3(0), vec2(1), 0, vec4(1, 0.5f, 0, 0.2f));
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "Trigger";
}

Trigger::~Trigger()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
}

vec2 Trigger::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

    drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &editorRotation, Editor::panelPropertiesX, strID + "ori").y;
    drawPos.y -= Editor::DrawProperty(drawPos, "Scale", &editorSize, Editor::panelPropertiesX, strID + "size").y;

	drawPos.y -= Editor::CheckBox(drawPos, "Trigger once", &once, Editor::panelPropertiesX).y;

	drawPos.y -= onEnter.DrawInPanel(drawPos, "On player enter").y;
	drawPos.y -= onExit.DrawInPanel(drawPos, "On player exit").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* Trigger::Copy()
{
	Trigger* newObj = new Trigger(*this);

	// copy collider
	RectCollider* oldCollider = (RectCollider*)this->clickCollider;
	newObj->clickCollider = new RectCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->orientation, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	return newObj;
}

void Trigger::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

    EditorSaveManager::FloatProp(props, "orientation", &editorRotation);
    editorSize = EditorSaveManager::StringToVector2((*props)["scale"], vec2(1));

	once = (*props)["once"] == "1";

	EventList::Load(&onEnter, (*props)["onEnter"]);
	EventList::Load(&onExit, (*props)["onExit"]);
}

void Trigger::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("orientation", editorRotation);
    EditorSaveManager::WriteProp("scale", editorSize);

    EditorSaveManager::WriteProp("onEnter", onEnter.GetString());
    EditorSaveManager::WriteProp("onExit", onExit.GetString());

    EditorSaveManager::WriteProp("once", once);
}

void Trigger::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void Trigger::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void Trigger::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = events;
	*resCount = TRIGGER_EVENT_COUNT;
}

void Trigger::OnMainLoop()
{
	EditorObject::OnMainLoop();
	if (!enabled) return;
    
	if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = GetEditScale();
		editorSprite->rotate = GetEditRotation();
	}
	((RectCollider*)clickCollider)->size = GetEditScale();
	((RectCollider*)clickCollider)->orientation = GetEditRotation();

	if (!Editor::enabled && Player::ingameInstance != nullptr)
	{
		if (!once || !hasAlredyTriggered)
		{
			vec3 res = Player::ingameInstance->collider->CollideWith((RectCollider*)clickCollider);

			if (res.z != 0) // Is colliding
			{
				if (!collideWithPlayer)
				{
					hasAlredyTriggered = true;
					collideWithPlayer = true;
					onEnter.Call(this);
				}
			}
			else
			{
				if (collideWithPlayer)
				{
					collideWithPlayer = false;
					onExit.Call(this);
				}
			}
		}
	}
	else
	{
		collideWithPlayer = false;
	}
}
