#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "PhysicObject.h"

constexpr int OBJECT_FOLLOWER_EVENT_COUNT = 2;

using namespace glm;

class ObjectFollower : public EditorObject, PhysicObject
{
public:
	ObjectFollower();
	~ObjectFollower();

	static ObjectEvent events[OBJECT_FOLLOWER_EVENT_COUNT];

	int targetID = -1;
	bool followPlayer = false;

	float acceleration = 1; // u.s-2
	float maxSpeed = 2; // u.s-1
	float drag = 0.8f; // u.s-2
	float targetRadius = 0.2f; // If distance to target is less than that, stop acceleration

	bool rotateTowardsTarget = true;
	float rotateSpeed = 180; // deg.s-1

	bool collideWithPhysics = false;
	float bounce = 0.3f; // force is multiplied by this after collision
	float colliderSize = 1;

	bool mustFollow = true;

    Sprite* editorSprite = nullptr;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	virtual void OnBeforeMove() override;
	virtual void OnAfterMove() override;

	virtual void OnSelected() override;
	virtual void OnUnselected() override;

private:
	EditorObject* _target = nullptr;
	CircleCollider* collideCollider;
	Sprite* colliderSprite;
};

