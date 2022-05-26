#include "LogicRelay.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "TweenManager.h"

const ObjectEvent LogicRelay::events[LOGIC_RELAY_EVENT_COUNT] = {
	ObjectEvent{
		"Trigger",
		[](EditorObject* object, void* params) { ((LogicRelay*)object)->Trigger(); }
	},
};

LogicRelay::LogicRelay() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\relay.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "LogicRelay";
}

LogicRelay::~LogicRelay()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }   
}

vec2 LogicRelay::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::CheckBox(drawPos, "Trigger on start", &triggerOnStart, Editor::panelPropertiesX).y;
	drawPos.y -= onTrigger.DrawInPanel(drawPos, "On trigger").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* LogicRelay::Copy()
{
	LogicRelay* newObj = new LogicRelay(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->position, oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void LogicRelay::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	EventList::Load(&onTrigger, (*props)["onTrigger"]);
	triggerOnStart = (*props)["triggerOnStart"] == "1";

	// Wait one second before sending auto trigger
	if (triggerOnStart && !Editor::enabled)
	{
		EventManager::DoInOneFrame([this] {this->Trigger(); });
	}
}

void LogicRelay::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("onTrigger", onTrigger.GetString());
	EditorSaveManager::WriteProp("triggerOnStart", triggerOnStart);
}

void LogicRelay::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void LogicRelay::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void LogicRelay::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = &events[0];
	*resCount = LOGIC_RELAY_EVENT_COUNT;
}

void LogicRelay::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}

void LogicRelay::Trigger()
{
	onTrigger.Call(this);
}
