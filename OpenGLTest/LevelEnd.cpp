#include "LevelEnd.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "TweenManager.h"
#include "LightManager.h"

const ObjectEvent LevelEnd::events[LEVEL_END_EVENT_COUNT] =
{
	ObjectEvent {
		"End level",
		[](EditorObject* object, void* param) {
			((LevelEnd*)object)->EndLevel();
		},
	}
};

LevelEnd::LevelEnd() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("engine\\end.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "LevelEnd";
}

LevelEnd::~LevelEnd()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
}

vec2 LevelEnd::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::FileSelector(drawPos, "Next level", &nextLevel, &Editor::mapFiles, Editor::panelPropertiesX, strID + "nextLevel").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* LevelEnd::Copy()
{
	LevelEnd* newObj = new LevelEnd(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void LevelEnd::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);
	nextLevel = (*props)["nextLevel"];
}

void LevelEnd::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("nextLevel", nextLevel);
}

void LevelEnd::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void LevelEnd::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void LevelEnd::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = editorPosition;
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}

void LevelEnd::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = &(events[0]);
	*resCount = LEVEL_END_EVENT_COUNT;
}

void LevelEnd::EndLevel()
{
	if (isEndingLevel) return;
	isEndingLevel = true;
	EditorSaveManager::LoadLevelWithTransition(nextLevel);
}
