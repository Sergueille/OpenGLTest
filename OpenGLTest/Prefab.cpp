#include "Prefab.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

Prefab::Prefab() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(nullptr, vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "Prefab";
}

Prefab::~Prefab()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
}

vec2 Prefab::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* Prefab::Copy()
{
	Prefab* newObj = new Prefab(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->position, oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void Prefab::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);
}

void Prefab::Save()
{
	EditorObject::Save();
}

void Prefab::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void Prefab::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void Prefab::UpdateTransform()
{
	EditorObject::UpdateTransform();
    if (editorSprite != nullptr) editorSprite->position = editorPosition;
}
