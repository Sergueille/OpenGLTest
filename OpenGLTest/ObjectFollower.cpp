#include "ObjectFollower.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

ObjectFollower::ObjectFollower() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\objectFollower.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "ObjectFollower";
}

ObjectFollower::~ObjectFollower()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
        
}

vec2 ObjectFollower::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

    drawPos.y -= Editor::DrawProperty(drawPos, "Start orientation", &editorRotation, Editor::panelPropertiesX, strID + "ori").y + Editor::margin;

	_target = targetID == -1 ? nullptr : Editor::GetEditorObjectByIDInObjectContext(this, targetID, true, false);
    drawPos.y -= Editor::ObjectSelector(drawPos, "Target", &_target, Editor::panelPropertiesX, strID + "target").y + Editor::margin;
	targetID = _target == nullptr ? -1 : _target->ID;

	drawPos.y -= Editor::DrawProperty(drawPos, "Acceleration", &acceleration, Editor::panelPropertiesX, strID + "acceleration").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Max speed", &maxSpeed, Editor::panelPropertiesX, strID + "maxSpeed").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Drag", &drag, Editor::panelPropertiesX, strID + "drag").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Target radius", &targetRadius, Editor::panelPropertiesX, strID + "targetRadius").y + Editor::margin;

	drawPos.y -= Editor::CheckBox(drawPos, "Rotate towards target", &rotateTowardsTarget, Editor::panelPropertiesX).y;
	if (rotateTowardsTarget)
	{
		drawPos.y -= Editor::DrawProperty(drawPos, "Rotate speed", &rotateSpeed, Editor::panelPropertiesX, strID + "rotateSpeed").y;
	}
	drawPos.y -= Editor::margin;

	drawPos.y -= Editor::CheckBox(drawPos, "Collide with phys.", &collideWithPhysics, Editor::panelPropertiesX).y;
	if (rotateTowardsTarget)
	{
		drawPos.y -= Editor::DrawProperty(drawPos, "Bounce", &bounce, Editor::panelPropertiesX, strID + "bounce").y;
	}

	drawPos.y -= Editor::CheckBox(drawPos, "Must follow", &mustFollow, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* ObjectFollower::Copy()
{
	ObjectFollower* newObj = new ObjectFollower(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	return newObj;
}

void ObjectFollower::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

    EditorSaveManager::FloatProp(props, "orientation", &editorRotation);

    EditorSaveManager::FloatProp(props, "acceleration", &acceleration);
    EditorSaveManager::FloatProp(props, "maxSpeed", &maxSpeed);
    EditorSaveManager::FloatProp(props, "drag", &drag);
    EditorSaveManager::FloatProp(props, "targetRadius", &targetRadius);

	rotateTowardsTarget = (*props)["rotateTowardsTarget"] == "1";
    EditorSaveManager::FloatProp(props, "rotateSpeed", &rotateSpeed);

	collideWithPhysics = (*props)["collideWithPhysics"] == "1";
	EditorSaveManager::FloatProp(props, "bounce", &bounce);

	mustFollow = (*props)["mustFollow"] == "1";

    EditorSaveManager::IntProp(props, "targetID", &targetID);
	if (!Editor::enabled)
	{
		_target = targetID == -1 ? nullptr : Editor::GetEditorObjectByIDInObjectContext(this, targetID, false, false);

		if (_target == nullptr)
			fprintf(stderr, "Object follower (%s) has no target!", this->name);
	}
}

void ObjectFollower::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("orientation", editorRotation);
    EditorSaveManager::WriteProp("targetID", targetID);
    EditorSaveManager::WriteProp("acceleration", acceleration);
    EditorSaveManager::WriteProp("maxSpeed", maxSpeed);
    EditorSaveManager::WriteProp("drag", drag);
    EditorSaveManager::WriteProp("targetRadius", targetRadius);
    EditorSaveManager::WriteProp("rotateTowardsTarget", rotateTowardsTarget);
    EditorSaveManager::WriteProp("rotateSpeed", rotateSpeed);
    EditorSaveManager::WriteProp("collideWithPhysics", collideWithPhysics);
    EditorSaveManager::WriteProp("bounce", bounce);
    EditorSaveManager::WriteProp("mustFollow", mustFollow);
}

void ObjectFollower::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void ObjectFollower::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void ObjectFollower::OnMainLoop()
{
	EditorObject::OnMainLoop();
	if (!enabled) return;
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	if (!Editor::enabled)
	{
		if (mustFollow && _target != nullptr)
		{
			vec2 delta = _target->GetEditPos() - GetEditPos();
			float sqrDist = Utility::SqrLength(delta);

			if (sqrDist > targetRadius * targetRadius)
			{
				velocity += glm::normalize(delta) * acceleration * GetDeltaTime(); // Acceleration

				if (Utility::SqrLength(velocity) > maxSpeed * maxSpeed) // Sound clamp velocity
				{
					velocity = glm::normalize(velocity) * maxSpeed;
				}
			}
		}

		// Drag
		if (drag * GetDeltaTime() > glm::length(velocity))
		{
			velocity = vec2(0);
		}
		else
		{
			velocity -= glm::normalize(velocity) * drag * GetDeltaTime();
		}

		// Apply velocity
		SetGlobalEditPos(GetEditPos() + vec3(velocity.x, velocity.y, 0) * GetDeltaTime());
	}
}
