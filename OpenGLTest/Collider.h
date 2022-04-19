#pragma once

#include <list>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Sprite.h"

class CircleCollider;
class RectCollider;
class Collider
{
	friend class PhysicObject;

public:
	static std::list<CircleCollider*> circleColliders;
	static std::list<RectCollider*> rectColliders;

	/// <summary>
	/// The position of the collider
	/// </summary>
	glm::vec2 position;

	bool enabled = true;

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

protected:
	/// <summary>
	/// Shold this collider collide with physic objects
	/// </summary>
	bool collideWithPhys;
};
