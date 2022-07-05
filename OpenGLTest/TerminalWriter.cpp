#include "TerminalWriter.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "LocalizationManager.h"
#include "TerminalManager.h"

ObjectEvent TerminalWriter::events[TERM_WRIT_EVENT_COUNT] = {
	ObjectEvent {
		"Write",
		[](EditorObject* object, void* param) { ((TerminalWriter*)object)->Write(); },
	},
	ObjectEvent {
		"Clear terminal",
		[](EditorObject* object, void* param) { TerminalManager::ClearTerminal(); },
	},
};

TerminalWriter::TerminalWriter() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\terminalWriter.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "TerminalWriter";
}

TerminalWriter::~TerminalWriter()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }  
}

vec2 TerminalWriter::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);
	bool pressed;

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;
	drawPos.y -= Editor::CheckBox(drawPos, "Random line", &writeRandom, Editor::panelPropertiesX).y;
	drawPos.y -= Editor::margin;

	for (int i = 0; i < keys.size(); i++)
	{
		std::string stri = std::to_string(i);
		drawPos.y -= Editor::DrawProperty(drawPos, stri, &keys[i], Editor::panelPropertiesX, strID + stri).y;
		drawPos.y -= TextManager::RenderText(LocalizationManager::GetLocale(keys[i]), drawPos, Editor::textSize).y;

		drawPos.y -= Editor::UIButton(drawPos, "Remove", &pressed).y;
		if (pressed)
		{ 
			keys.erase(keys.begin() + i);
			i--;
		}

		drawPos.y -= Editor::margin;
	}

	drawPos.y -= Editor::UIButton(drawPos, "Add", &pressed).y;
	if (pressed)
	{
		keys.push_back("");
	}

	drawPos.y -= onFinished.DrawInPanel(drawPos, "On finished").y;
    
	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* TerminalWriter::Copy()
{
	TerminalWriter* newObj = new TerminalWriter(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	return newObj;
}

void TerminalWriter::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	int count;
	EditorSaveManager::IntProp(props, "count", &count);
    
	keys.clear();
	for (int i = 0; i < count; i++)
	{
		keys.push_back((*props)["key" + std::to_string(i)]);
	}

	EventList::Load(&onFinished, (*props)["onFinished"]);

	writeRandom = (*props)["writeRandom"] == "1";
}

void TerminalWriter::Save()
{
	EditorObject::Save();

	int count = (int)keys.size();
	EditorSaveManager::WriteProp("count", count);
	for (int i = 0; i < count; i++)
	{
		EditorSaveManager::WriteProp("key" + std::to_string(i), keys[i]);
	}

	EditorSaveManager::WriteProp("onFinished", onFinished.GetString());	

	EditorSaveManager::WriteProp("writeRandom", writeRandom);
}

void TerminalWriter::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void TerminalWriter::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void TerminalWriter::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = events;
	*resCount = TERM_WRIT_EVENT_COUNT;
}

void TerminalWriter::Write()
{
	nextWriteTime = 0;
	shouldWrite = true;
	writePos = 0;
}

void TerminalWriter::OnMainLoop()
{
	EditorObject::OnMainLoop();
	if (!enabled) return;
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	if (shouldWrite)
	{
		if (!writeRandom && Utility::time > nextWriteTime)
		{
			if (writePos >= keys.size())
			{
				shouldWrite = false;
				onFinished.Call(this);
			}
			else
			{
				TerminalManager::Write(keys[writePos]);
				nextWriteTime = Utility::time + ((float)LocalizationManager::GetLocale(keys[writePos]).length() / (float)charPerSec) + timeBetweenLines;
				writePos++;
			}
		}
		else if (writeRandom)
		{
			shouldWrite = false;
			TerminalManager::Write(keys[rand() % keys.size()]);
		}
	}
}
 