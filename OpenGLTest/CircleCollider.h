#pragma once
#include "Collider.h"
#include "RectCollider.h"

#include <math.h>

using namespace glm;

class CircleCollider : public Collider
{
public:
	CircleCollider(vec2 position, float size, bool collideWithPhys = true);
	~CircleCollider();

	/// <summary>
	/// The diameter of the collider
	/// </summary>
	float size;

	virtual vec3 CollideWith(CircleCollider* other) override;
	virtual vec3 CollideWith(RectCollider* other) override;

	virtual std::vector<vec2> RaycastPoints(float a, float b) override;

	virtual void SetCollideWithPhys(bool value) override;
};
