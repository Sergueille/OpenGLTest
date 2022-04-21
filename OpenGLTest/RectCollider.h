#pragma once
#include "Collider.h"
#include "CircleCollider.h"

#include <vector>

using namespace glm;

class RectCollider : public Collider
{
public:
	RectCollider(vec2 position, vec2 size, float orientation, bool collideWithPhys = true);
	~RectCollider();

	/// <summary>
	/// The size of the collider
	/// </summary>
	vec2 size;
	/// <summary>
	/// Rotation of the collider, 0 is normal, trigonometric (counter-clockwise)
	/// </summary>
	float orientation;

	virtual vec3 CollideWith(CircleCollider* other) override;
	virtual vec3 CollideWith(RectCollider* other) override;

	virtual void SetCollideWithPhys(bool value) override;

	/// <summary>
	/// Get corners of collider, to-right first
	/// </summary>
	std::vector<vec2> GetPoints();
};

