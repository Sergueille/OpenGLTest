#include "ObjectFollower.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "Player.h"

ObjectEvent ObjectFollower::events[OBJECT_FOLLOWER_EVENT_COUNT] = {
	ObjectEvent {
		"Start following",
		[](EditorObject* object, void* param) {
			((ObjectFollower*)object)->StartFollowing();
		}
	},
	ObjectEvent {
		"Stop following",
		[](EditorObject* object, void* param) {
			((ObjectFollower*)object)->StopFollowing();
		}
	},
};

ObjectFollower::ObjectFollower() : EditorObject(vec3(0)), PhysicObject(collideCollider)
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\objectFollower.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();

		colliderSprite = new Sprite(nullptr, GetEditPos(), vec2(1), 0.0f, vec4(0.5, 1, 0, 0.2));
    }

	collideCollider = new CircleCollider(vec2(0), 0, false);
	collider = collideCollider;
	physicsEnabled = false;
	useGravity = false;
            
	typeName = "ObjectFollower";
}

ObjectFollower::~ObjectFollower()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	if (collideCollider != nullptr)
	{
		delete collideCollider;
		collideCollider = nullptr;
		collider = nullptr;
	}

	if (colliderSprite != nullptr)
	{
		delete colliderSprite;
		colliderSprite = nullptr;
	}
}

vec2 ObjectFollower::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

    drawPos.y -= Editor::DrawProperty(drawPos, "Start orientation", &editorRotation, Editor::panelPropertiesX, strID + "ori").y + Editor::margin;

	if (!followPlayer)
	{
		_target = targetID == -1 ? nullptr : Editor::GetEditorObjectByIDInObjectContext(this, targetID, true, false);
		drawPos.y -= Editor::ObjectSelector(drawPos, "Target", &_target, Editor::panelPropertiesX, strID + "target").y;
		targetID = _target == nullptr ? -1 : _target->ID;
	}
	drawPos.y -= Editor::CheckBox(drawPos, "Follow player", &followPlayer, Editor::panelPropertiesX).y + Editor::margin;

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
		drawPos.y -= Editor::DrawProperty(drawPos, "Collider size", &colliderSize, Editor::panelPropertiesX, strID + "size").y + Editor::margin;
	}

	drawPos.y -= Editor::CheckBox(drawPos, "Must follow", &mustFollow, Editor::panelPropertiesX).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Stop follow dist.", &stopFollowingAfterDistance, Editor::panelPropertiesX, strID + "stopFollowingAfterDistance").y;

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
    if (colliderSprite != nullptr) newObj->colliderSprite = this->colliderSprite->Copy();

    if (collideCollider != nullptr) 
		newObj->collideCollider = new CircleCollider(collideCollider->GetPos(), collideCollider->size, true);
	newObj->_target = nullptr;

	newObj->SubscribeToMainLoop();

	return newObj;
}

void ObjectFollower::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

    EditorSaveManager::FloatProp(props, "orientation", &editorRotation);
	while (editorRotation > 360) editorRotation -= 360;
	while (editorRotation < 0) editorRotation += 360;
    EditorSaveManager::FloatProp(props, "colliderSize", &colliderSize);

    EditorSaveManager::FloatProp(props, "acceleration", &acceleration);
    EditorSaveManager::FloatProp(props, "maxSpeed", &maxSpeed);
    EditorSaveManager::FloatProp(props, "drag", &drag);
    EditorSaveManager::FloatProp(props, "targetRadius", &targetRadius);

	rotateTowardsTarget = (*props)["rotateTowardsTarget"] == "1";
    EditorSaveManager::FloatProp(props, "rotateSpeed", &rotateSpeed);

	collideWithPhysics = (*props)["collideWithPhysics"] == "1";

	mustFollow = (*props)["mustFollow"] == "1";
	startMustFollow = mustFollow;

	EditorSaveManager::FloatProp(props, "stopFollowingAfterDistance", &stopFollowingAfterDistance);

    EditorSaveManager::IntProp(props, "targetID", &targetID);
	followPlayer = (*props)["followPlayer"] == "1";
	if (!Editor::enabled)
	{
		if (followPlayer)
		{
			_target = (EditorObject*)Player::ingameInstance;
		}
		else
		{
			_target = targetID == -1 ? nullptr : Editor::GetEditorObjectByIDInObjectContext(this, targetID, false, false);
		}

		if (_target == nullptr)
			fprintf(stderr, "Object follower (%s) has no target!", this->name.c_str());
	}

	physicsEnabled = false;
	collideCollider->SetCollideWithPhys(!Editor::enabled);
	SetPos(GetEditPos());

	startPos = GetLocalEditPos();
	startRot = GetLocalEditRotation();
}

void ObjectFollower::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("orientation", editorRotation);
    EditorSaveManager::WriteProp("editorSize", editorSize);
    EditorSaveManager::WriteProp("targetID", targetID);
    EditorSaveManager::WriteProp("followPlayer", followPlayer);
    EditorSaveManager::WriteProp("acceleration", acceleration);
    EditorSaveManager::WriteProp("maxSpeed", maxSpeed);
    EditorSaveManager::WriteProp("drag", drag);
    EditorSaveManager::WriteProp("targetRadius", targetRadius);
    EditorSaveManager::WriteProp("rotateTowardsTarget", rotateTowardsTarget);
    EditorSaveManager::WriteProp("rotateSpeed", rotateSpeed);
    EditorSaveManager::WriteProp("collideWithPhysics", collideWithPhysics);
    EditorSaveManager::WriteProp("mustFollow", mustFollow);
    EditorSaveManager::WriteProp("colliderSize", colliderSize);
    EditorSaveManager::WriteProp("stopFollowingAfterDistance", stopFollowingAfterDistance);
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

void ObjectFollower::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = events;
	*resCount = OBJECT_FOLLOWER_EVENT_COUNT;
}

void ObjectFollower::OnBeforeMove()
{
	if (!physicsEnabled) return;

	SetPos(vec2(GetEditPos()));

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

		if (rotateTowardsTarget)
		{
			while (editorRotation > 360) editorRotation -= 360; // Make sure is less than 360

			float targetAngle = Utility::GetVectorAngle(velocity);
			float angleDist = targetAngle - editorRotation;

			if (angleDist > 180)
			{
				editorRotation += 360;
				angleDist = 360 - angleDist;
			}
			if (angleDist < -180)
			{
				editorRotation -= 360;
				angleDist = 360 - angleDist;
			}

			if (targetAngle > editorRotation)
				editorRotation += AbsMin(angleDist, rotateSpeed * GetDeltaTime());
			else
				editorRotation -= AbsMin(angleDist, rotateSpeed * GetDeltaTime());
		}

		// Too far
		if (stopFollowingAfterDistance > 0 && sqrDist > stopFollowingAfterDistance * stopFollowingAfterDistance)
		{
			StopFollowing();
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
}

void ObjectFollower::OnAfterMove()
{
	SetGlobalEditPos(vec3(GetPos().x, GetPos().y, GetEditPos().z));
}

void ObjectFollower::OnSelected()
{
	colliderSprite->DrawOnMainLoop();
}

void ObjectFollower::OnUnselected()
{
	colliderSprite->StopDrawing();
}

void ObjectFollower::ResetIngameState()
{
	if (startMustFollow)
		StartFollowing();
	else
		StopFollowing();

	SetEditPos(startPos);
	SetEditRotation(startRot);
}

void ObjectFollower::StartFollowing()
{
	mustFollow = true;
	physicsEnabled = true;
}

void ObjectFollower::StopFollowing()
{
	mustFollow = false;
	physicsEnabled = false;
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

	if (colliderSprite != nullptr)
	{
		colliderSprite->position = GetEditPos();
		colliderSprite->size = GetEditScale();
		colliderSprite->rotate = GetEditRotation();
	}

	if (collideWithPhysics)
	{
		collideCollider->SetPos(vec2(GetPos()));
		collideCollider->size = colliderSize;
	}
}
