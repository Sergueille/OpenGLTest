#include "RectCollider.h"
#include "Collider.h"
#include "Utility.h"
#include <math.h>

using namespace glm;

RectCollider::RectCollider(vec2 position, vec2 size, float orientation, bool collideWithPhys)
{
	this->position = position;
	this->size = size;
	this->orientation = orientation;

	this->collideWithPhys = collideWithPhys;
	if (collideWithPhys)
		Collider::rectColliders.push_back(this);
}

RectCollider::~RectCollider()
{
	if (collideWithPhys)
		Collider::rectColliders.remove(this);
}

vec3 RectCollider::CollideWith(CircleCollider* other)
{
	if (!enabled) return vec3(0, 0, 0);

	vec3 res = other->CollideWith(this);
	res.x = -res.x;
	res.y = -res.y;
	return res;
}

vec3 RectCollider::CollideWith(RectCollider* other)
{
	if (!enabled) return vec3(0, 0, 0);

	throw "Not implemented!";
}

std::vector<vec2> RectCollider::RaycastPoints(float ra, float rb)
{
	std::vector<vec2> points = GetPoints();
	std::vector<vec2> result = std::vector<vec2>();

	// For each side
	for (auto it = points.begin(); it != points.end(); it++)
	{
		vec2 point = *it; // First point pf the side
		vec2 next; // Second point of the side

		if (it + 1 == points.end()) next = points[0];
		else next = *(it + 1);

		float a, b;
		GetLineEquationFromPoints(point, next, &a, &b); // Get line equation

		if (ra == a || (std::isnan(ra) && std::isnan(a))) continue; // Continue if lines are not crossing

		vec2 res = LineItersection(ra, rb, a, b); // Get intersection points

		// Check if the intesection point is on the line
		if (point.x == next.x) // Vectical lines
		{
			if ((point.y < next.y && res.y > point.y && res.y < next.y)
				|| (point.y > next.y && res.y < point.y && res.y > next.y))
			{
				result.push_back(res);
			}
		}
		else // Other lines
		{
			if ((point.x < next.x && res.x > point.x && res.x < next.x)
				|| (point.x > next.x && res.x < point.x && res.x > next.x))
			{
				result.push_back(res);
			}
		}
	}

	return result;
}

void RectCollider::SetCollideWithPhys(bool value)
{
	if (collideWithPhys != value)
	{
		if (collideWithPhys)
			Collider::rectColliders.remove(this);
		else
			Collider::rectColliders.push_back(this);
	}

	collideWithPhys = value;
}

std::vector<vec2> RectCollider::GetPoints()
{
	vec2 halfSize = size / 2.f;
	vec2 xVect = Utility::Rotate(vec2(1, 0), orientation) * halfSize.x;
	vec2 yVect = Utility::Rotate(vec2(0, 1), orientation) * halfSize.y;

	return {
		position + xVect + yVect,
		position - xVect + yVect,
		position - xVect - yVect,
		position + xVect - yVect,
	};
}
