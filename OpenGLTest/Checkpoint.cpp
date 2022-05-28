#include "Checkpoint.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

ObjectEvent Checkpoint::events[CHECKPOINT_EVENT_COUNT] = {
	ObjectEvent {
		"Save game",
		[](EditorObject* object, void* param) { ((Checkpoint*)object)->SaveGame(); },
	},
};

Checkpoint::Checkpoint() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\checkpoint.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "Checkpoint";
}

Checkpoint::~Checkpoint()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
}

vec2 Checkpoint::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* Checkpoint::Copy()
{
	Checkpoint* newObj = new Checkpoint(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void Checkpoint::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);
}

void Checkpoint::Save()
{
	EditorObject::Save();
}

void Checkpoint::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void Checkpoint::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void Checkpoint::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = &events[0];
	*resCount = CHECKPOINT_EVENT_COUNT;
}

void Checkpoint::SaveGame()
{
	if (!Editor::enabled)
		EditorSaveManager::SaveUserSave("Saves\\" + EditorSaveManager::currentUserSave);
}

void Checkpoint::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}
