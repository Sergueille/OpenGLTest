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
	ObjectEvent{
		"Set to alternative events",
		[](EditorObject* object, void* params) { ((LogicRelay*)object)->isAlt = true; }
	},
	ObjectEvent{
		"Set to normal events",
		[](EditorObject* object, void* params) { ((LogicRelay*)object)->isAlt = false; }
	},
	ObjectEvent{
		"Toogle alternative events",
		[](EditorObject* object, void* params) { ((LogicRelay*)object)->isAlt = !((LogicRelay*)object)->isAlt; }
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

	if (waitAction != nullptr && !waitAction->IsFinshedAt(Utility::time))
		TweenManager<float>::Cancel(waitAction);
}

vec2 LogicRelay::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::CheckBox(drawPos, "Trigger on start", &triggerOnStart, Editor::panelPropertiesX).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Delay", &delay, Editor::panelPropertiesX, strID + "delay").y;
	drawPos.y -= onTrigger.DrawInPanel(drawPos, "On trigger").y;
	drawPos.y -= onAltTrigger.DrawInPanel(drawPos, "On alternative trigger").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* LogicRelay::Copy()
{
	LogicRelay* newObj = new LogicRelay(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	// Wait one frame before sending auto trigger
	if (newObj->triggerOnStart && !Editor::enabled)
	{
		EventManager::DoInOneFrame([newObj] { newObj->Trigger(); });
	}

	return newObj;
}

void LogicRelay::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	EventList::Load(&onTrigger, (*props)["onTrigger"]);
	EventList::Load(&onAltTrigger, (*props)["onAltTrigger"]);
	triggerOnStart = (*props)["triggerOnStart"] == "1";

	EditorSaveManager::FloatProp(props, "delay", &delay);

	// Wait one frame before sending auto trigger
	if (triggerOnStart && !Editor::enabled && enabled)
	{
		EventManager::DoInOneFrame([this] { this->Trigger(); });
	}
}

void LogicRelay::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("onTrigger", onTrigger.GetString());
	EditorSaveManager::WriteProp("onAltTrigger", onAltTrigger.GetString());
	EditorSaveManager::WriteProp("triggerOnStart", triggerOnStart);
	EditorSaveManager::WriteProp("delay", delay);
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
	if (!enabled) return;

	if (delay > 0)
	{
		if (waitAction != nullptr && !waitAction->IsFinshedAt(Utility::time))
			TweenManager<float>::Cancel(waitAction);

		TweenManager<float>::Tween(0, 1, delay, [](float value) {}, linear)
		->SetOnFinished([this] { 
			TriggerNow();
		});
	}
	else
	{
		TriggerNow();
	}
}

void LogicRelay::TriggerNow()
{
	if (Editor::enabled) return;

	if (isAlt)
		onAltTrigger.Call(this);
	else
		onTrigger.Call(this);
}
