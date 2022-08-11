#include "TemperatureManager.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "LocalizationManager.h"
#include "TerminalManager.h"

ObjectEvent TemperatureManager::events[TEMP_MANAGER_EVENT_COUNT] = {
	ObjectEvent {
		"Start",
		[](EditorObject* obj, void* param) { ((TemperatureManager*)obj)->Start(); },
	},
	ObjectEvent {
		"Sub",
		[](EditorObject* obj, void* param) { ((TemperatureManager*)obj)->Sub(); },
	},
	ObjectEvent {
		"Stop",
		[](EditorObject* obj, void* param) { ((TemperatureManager*)obj)->Stop(); },
	},
};

TemperatureManager::TemperatureManager() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\temperatureManager.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "TemperatureManager";
}

TemperatureManager::~TemperatureManager()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
        
}

vec2 TemperatureManager::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Start temperature", &startTemperature, Editor::panelPropertiesX, strID + "startTemperature").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Max temperature", &maxTemperature, Editor::panelPropertiesX, strID + "maxTemperature").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Duration", &duration, Editor::panelPropertiesX, strID + "duration").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Sub amount", &subAmount, Editor::panelPropertiesX, strID + "subAmount").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Sub duration", &subDuration, Editor::panelPropertiesX, strID + "subDuration").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "TextColor", &textColor, Editor::panelPropertiesX, strID + "textColor", true).y + Editor::margin;

	drawPos.y -= Editor::DrawProperty(drawPos, "Text", &localKey, Editor::panelPropertiesX, strID + "localKey").y;
	drawPos.y -= TextManager::RenderText(LocalizationManager::GetLocale(localKey), drawPos, Editor::textSize).y + Editor::margin;

	drawPos.y -= onReachMax.DrawInPanel(drawPos, "On reach max").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* TemperatureManager::Copy()
{
	TemperatureManager* newObj = new TemperatureManager(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	return newObj;
}

void TemperatureManager::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	EditorSaveManager::IntProp(props, "startTemperature", &startTemperature);
	EditorSaveManager::IntProp(props, "maxTemperature", &maxTemperature);
	EditorSaveManager::IntProp(props, "duration", &duration);
	EditorSaveManager::IntProp(props, "subAmount", &subAmount);
	EditorSaveManager::IntProp(props, "subDuration", &subDuration);
	textColor = EditorSaveManager::StringToVector3((*props)["textColor"], vec3(1));
	localKey = (*props)["localKey"];
	EventList::Load(&onReachMax, (*props)["onReachMax"]);
}

void TemperatureManager::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("startTemperature", startTemperature);
	EditorSaveManager::WriteProp("maxTemperature", maxTemperature);
	EditorSaveManager::WriteProp("duration", duration);
	EditorSaveManager::WriteProp("subAmount", subAmount);
	EditorSaveManager::WriteProp("subDuration", subDuration);
	EditorSaveManager::WriteProp("localKey", localKey);
	EditorSaveManager::WriteProp("textColor", textColor);
	EditorSaveManager::WriteProp("onReachMax", onReachMax.GetString());
}

void TemperatureManager::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void TemperatureManager::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void TemperatureManager::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = events;
	*resCount = TEMP_MANAGER_EVENT_COUNT;
}

void TemperatureManager::Start()
{
	if (isActive) return;

	isActive = true;
	currentTemp = static_cast<float>(startTemperature);
	startTime = Utility::time;
	eventAlreadySent = false;
}

void TemperatureManager::Sub()
{

}

void TemperatureManager::Stop()
{
	isActive = false;
}

void TemperatureManager::OnMainLoop()
{
	EditorObject::OnMainLoop();
	if (!enabled) return;
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	if (isActive)
	{
		// Get temperature
		float t = (Utility::time - startTime) / duration;

		currentTemp = Utility::Lerp(startTemperature, maxTemperature, t);

		if (currentTemp >= maxTemperature)
		{
			currentTemp = maxTemperature;

			if (!eventAlreadySent)
			{
				onReachMax.Call(this);
				eventAlreadySent = true;
			}
		}

		// Display
		std::string txt = LocalizationManager::GetLocale(localKey) + " " + std::to_string(currentTemp) + "°C";
		vec3 pos = vec3(
			TerminalManager::screenMargin, 
			TerminalManager::screenMargin, 
			TerminalManager::zPos);

		TextManager::RenderText(txt, pos, TerminalManager::textSize, TextManager::right, textColor, true);
	}
}
