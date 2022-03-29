#pragma once
#include "Collider.h"
#include "RectCollider.h"

#include <math.h>

using namespace glm;

class CircleCollider : public Collider
{
public:
	CircleCollider(vec2 position, float size);
	~CircleCollider();

	/// <summary>
	/// The diameter of the collider
	/// </summary>
	float size;

	virtual vec3 CollideWith(CircleCollider* other) override;
	virtual vec3 CollideWith(RectCollider* other) override;
};
