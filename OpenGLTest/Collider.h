#pragma once

#include <list>
#include <vector>
#include <glm/glm.hpp>

#include "Sprite.h"

class CircleCollider;
class RectCollider;
class Collider
{
	friend class PhysicObject;

public:
	Collider();
	virtual ~Collider();

	static std::list<CircleCollider*> circleColliders;
	static std::list<RectCollider*> rectColliders;

	/// <summary>
	/// The difference between the collider's position and it's position last frame
	/// </summary>
	glm::vec2 deltaPos = glm::vec2(0, 0);

	bool enabled = true;

	void SetPos(glm::vec2 value);
	glm::vec2 GetPos();

	/// <summary>
	/// Determines if is colliding with a specified CircleCollider
	/// </summary>
	/// <returns>A vector where X and Y represents normalisedcontact vector, and Z the distance to move objects to end collision</returns>
	virtual glm::vec3 CollideWith(CircleCollider* other) = 0;
	/// <summary>
	/// Determines if is colliding with a specified RectCollider
	/// </summary>
	/// <returns>A vector where X and Y represents normalisedcontact vector, and Z the distance to move objects to end collision</returns>
	virtual glm::vec3 CollideWith(RectCollider* other) = 0;

	/// <summary>
	/// Determine if this collider is touching any of the colliders that can collide with phisics
	/// </summary>
	bool IsTouchingAnyCollider();

	/// <summary>
	/// The raycast points of a line (ax+b)
	/// </summary>
	virtual std::vector<glm::vec2> RaycastPoints(float a, float b) = 0;

	/// <summary>
	/// RAYCAST!
	/// </summary>
	/// <returns></returns>
	static bool Raycast(glm::vec2 origin, glm::vec2 direction, glm::vec2* result, Collider* ignoreCollider = nullptr, Collider** hitCollider = nullptr);
	 
	virtual void SetCollideWithPhys(bool value) = 0;
	bool MustCollideWithPhys();

	virtual void GetAABB(glm::vec2* resMin, glm::vec2* resMax) = 0;

protected:
	/// <summary>
	/// Shold this collider collide with physic objects
	/// </summary>
	bool collideWithPhys;

	float lastSetPosTime = 0;

	/// <summary>
	/// The position of the collider
	/// </summary>
	glm::vec2 position;
};
