#include "Collider.h"
#include "RectCollider.h"
#include "CircleCollider.h"
#include "Utility.h"

std::list<CircleCollider*> Collider::circleColliders = std::list<CircleCollider*>();
std::list<RectCollider*> Collider::rectColliders = std::list<RectCollider*>();

Collider::~Collider() { }

bool Collider::IsTouchingAnyCollider()
{
	// For each rect collider
	for (auto coll = Collider::rectColliders.begin(); coll != Collider::rectColliders.end(); coll++)
	{
		if ((*coll)->collideWithPhys && (*coll)->enabled && (Collider*)*coll != this)
		{
			// Evaluate collision data
			vec3 res = this->CollideWith(*coll);

			if (res.z != 0)
				return true;
		}
	}

	// For each circle colliders
	for (auto coll = Collider::circleColliders.begin(); coll != Collider::circleColliders.end(); coll++)
	{
		if ((*coll)->collideWithPhys && (*coll)->enabled && (Collider*)*coll != this)
		{
			// Evaluate collision data
			vec3 res = this->CollideWith(*coll);

			if (res.z != 0)
				return true;
		}
	}

	return false;
}

bool Collider::Raycast(vec2 origin, vec2 direction, vec2* result, Collider* ignoreCollider)
{
	float a, b;
	Utility::GetLineEquationFromPoints(origin, origin + direction, &a, &b);

	vec2 nearestPoint;
	float nearestPointDist = FLT_MAX;
	bool found = false;

	// For each rect collider
	for (auto coll = Collider::rectColliders.begin(); coll != Collider::rectColliders.end(); coll++)
	{
		if ((*coll)->collideWithPhys && (*coll)->enabled && (Collider*)*coll != ignoreCollider)
		{
			std::vector<vec2> res = (*coll)->RaycastPoints(a, b);

			for (auto vec = res.begin(); vec != res.end(); vec++)
			{
				vec2 delta = (*vec) - origin;
				float dist = glm::length(delta);
				bool sameXsign = (delta.x * direction.x) >= 0;
				bool sameYsign = (delta.y * direction.y) >= 0;

				if (dist < nearestPointDist && sameXsign && sameYsign)
				{
					nearestPointDist = dist;
					nearestPoint = *vec;
					found = true;
				}
			}
		}
	}

	// For each circle colliders
	for (auto coll = Collider::circleColliders.begin(); coll != Collider::circleColliders.end(); coll++)
	{
		if ((*coll)->collideWithPhys && (*coll)->enabled && (Collider*)*coll != ignoreCollider)
		{
			std::vector<vec2> res = (*coll)->RaycastPoints(a, b);

			for (auto vec = res.begin(); vec != res.end(); vec++)
			{
				vec2 delta = (*vec) - origin;
				float dist = glm::length(delta);
				bool sameXsign = (delta.x * direction.x) >= 0;
				bool sameYsign = (delta.y * direction.y) >= 0;

				if (dist < nearestPointDist && sameXsign && sameYsign)
				{
					nearestPointDist = dist;
					nearestPoint = *vec;
					found = true;
				}
			}
		}
	}

	if (found)
	{
		*result = nearestPoint;
		return true;
	}
	else
	{
		return false;
	}

}

bool Collider::MustCollideWithPhys()
{
    return collideWithPhys;
}
