#pragma once

#include "Collider.h"

using namespace glm;

class PhysicObject
{
public:
	PhysicObject(Collider* coll);

	/// <summary>
	/// Max angle allowed btw normal and up vector to consideer the surface as walkable
	/// </summary>
	const float maxAngleForWalkableSurface = 50;
	/// <summary>
	/// g, in units.s-2
	/// </summary>
	const float gravityAcceleration = 50;

	vec2 velocity = vec2(0);
	bool isColliding = false;
	bool isOnWalkableSurface = false;

	Collider* collider = NULL;

	void Move();

	vec2 GetPos();
	void SetPos(vec2 pos);

private:
	vec2 position = vec2(0);

	void ReactToCollision(vec3 data);
};
