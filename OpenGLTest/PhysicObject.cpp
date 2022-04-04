#include "PhysicObject.h"

#include "Utility.h"
#include "EventManager.h"

using namespace glm;

PhysicObject::PhysicObject(Collider* coll)
{
	this->collider = coll;

	EventManager::OnMainLoop.push_back([this] { this->OnMainLoop(); }); // subscribe to the main loop
}

void PhysicObject::OnMainLoop()
{
	if (!physicsEnabled)
		return;

	this->OnBeforeMove();

	// Skip if no colliders
	if (collider == NULL)
		throw "Physic object has no collider! Please assign one to field 'collider' before moving.";

	// Skip if framerate too low
	if (Utility::GetDeltaTime() > 0.2f)
		return;

	// Gravity
	velocity.y -= gravityAcceleration * Utility::GetDeltaTime();

	// Move player
	SetPos(position + velocity * Utility::GetDeltaTime());

	isColliding = false;
	isOnWalkableSurface = false;

	// For each rect collider
	for (auto coll = Collider::rectColliders.begin(); coll != Collider::rectColliders.end(); coll++)
	{
		if ((void*)*coll != collider)
		{
			// Evaluate collision data
			vec3 res = collider->CollideWith(*coll);
			ReactToCollision(res);
		}
	}

	// For each circle colliders
	for (auto coll = Collider::circleColliders.begin(); coll != Collider::circleColliders.end(); coll++)
	{
		if ((void*)*coll != collider)
		{
			// Evaluate collision data
			vec3 res = collider->CollideWith(*coll);
			ReactToCollision(res);
		}
	}

	this->OnAfterMove();
}

vec2 PhysicObject::GetPos()
{
	return position;
}

void PhysicObject::SetPos(vec2 pos)
{
	position = pos;
	collider->position = pos;
}

void PhysicObject::OnBeforeMove()
{
}

void PhysicObject::OnAfterMove()
{
}

void PhysicObject::ReactToCollision(vec3 data)
{
	// If collision
	if (data.z != 0)
	{
		isColliding = true;

		// Get normal
		vec2 normal = vec2(data.x, data.y);

		if (!isOnWalkableSurface && Utility::AngleBetween(normal, vec2(0, 1)) < maxAngleForWalkableSurface)
			isOnWalkableSurface = true;

		// Move player on surface
		SetPos(position + normal * data.z);

		// If velocity not 0
		if (velocity.x != 0 || velocity.y != 0)
		{
			// Get reaction force and apply
			float angle = Utility::AngleBetween(velocity, normal) * Utility::DegToRad;
			velocity -= vec2(normal * glm::length(velocity) * cos(angle));
		}
	}
}

