#include "CircleCollider.h"
#include "Collider.h"

#include <algorithm>

#include "Utility.h"

using namespace glm;

CircleCollider::CircleCollider(vec2 position, float size, bool collideWithPhys)
{
	this->position = position;
	this->size = size;

	this->collideWithPhys = collideWithPhys;
	if (collideWithPhys)
		Collider::circleColliders.push_back(this);
}

CircleCollider::~CircleCollider()
{
	if (collideWithPhys)
		Collider::circleColliders.remove(this);
}

vec3 CircleCollider::CollideWith(CircleCollider* other)
{
	if (!enabled) return vec3(0, 0, 0);

	vec2 delta = position - other->position;
	float sqDist = (delta.x * delta.x) + (delta.y * delta.y);
	float maxDist = (size / 2) + (other->size / 2);
	float sqMaxDist = maxDist * maxDist;

	if (sqDist > sqMaxDist)
	{
		return vec3(0, 0, 0);
	}
	else
	{
		delta = glm::normalize(delta);
		return vec3(delta.x, delta.y, maxDist - std::sqrt(sqDist));
	}
}

vec3 CircleCollider::CollideWith(RectCollider* other)
{
	if (!enabled) return vec3(0, 0, 0);

	auto points = other->GetPoints();
	std::vector<vec3> projected = { vec3(), vec3(), vec3(), vec3() };
	int insideCount = 0;

	// For each side
	for (int i = 0; i < 4; i++)
	{
		// Project center of bass on side
		projected[i] = Utility::Project(points[i], points[(i+1)%4], position);

		// If projected point is inside the segment
		float sideSize = i%2 == 0 ? other->size.x : other->size.y;
		if (projected[i].z > -0.002 && projected[i].z <= sideSize + 0.002)
			insideCount += 1;
		else
			projected[i] = vec3(FLT_MAX); // Remove projected points that are ouside
	}

	vec2 nearest;
	bool inside = insideCount == 4;
	
	if (insideCount == 3)
	{
		throw "Shouldn't have this... There is a problem in the function";
	}
	else if (insideCount == 2 || insideCount == 4)
	{
		nearest = *std::max_element(projected.begin(), projected.end(), 
			[&](auto a, auto b) -> bool {
				return glm::length(vec2(a) - this->position) > glm::length(vec2(b) - this->position);
			}
		);
	}
	else if (insideCount == 1)
	{
		throw "Shouldn't have this... There is a problem in the function";
	}
	else if (insideCount == 0)
	{
		nearest = *std::max_element(points.begin(), points.end(),
			[&](auto a, auto b) -> bool {
				return glm::length(vec2(a) - this->position) > glm::length(vec2(b) - this->position);
			}
		);
	}

	vec2 delta = position - nearest;
	float sqDist = (delta.x * delta.x) + (delta.y * delta.y);
	float maxDist = (size / 2);
	float sqMaxDist = maxDist * maxDist;

	if (sqDist > sqMaxDist && !inside)
	{
		return vec3(0, 0, 0);
	}
	else
	{
		float dist;
		if (inside) {
			delta = -delta;
			dist = std::abs(maxDist + std::sqrt(sqDist));
		}
		else {
			dist = std::abs(maxDist - std::sqrt(sqDist));
		}

		delta = glm::normalize(delta);
		return vec3(delta.x, delta.y, dist);
	}
}

std::vector<vec2> CircleCollider::RaycastPoints(float a, float b)
{
	float x0 = position.x;
	float y0 = position.y;
	float r = size / 2;

	if (std::isnan(a)) // Vertical line
	{
		float _a = 1;
		float _b = -2 * y0;
		float _c = (y0 * y0) + ((b - x0) * (b - x0)) - (r * r);

		float delta = (_b * _b) - (4 * _a * _c);

		if (delta < 0)
		{
			return std::vector<vec2>();
		}
		else if (delta == 0)
		{
			float sy = (-_b) / (2 * _a);
			return std::vector<vec2> { vec2(b, sy) };
		}
		else
		{
			float s1y = (-_b - sqrt(delta)) / (2 * _a);
			float s2y = (-_b + sqrt(delta)) / (2 * _a);
			return std::vector<vec2> { vec2(b, s1y), vec2(b, s2y) };
		}
 	}
	else // Other lines
	{
		float _a = 1 + (a * a);
		float _b = -(2 * x0) + (2 * a * b) - (2 * y0 * a);
		float _c = (x0 * x0) + (b * b) - (2 * y0 * b) + (y0 * y0) - (r * r);

		float delta = (_b * _b) - (4 * _a * _c);

		if (delta < 0)
		{
			return std::vector<vec2>();
		}
		else if (delta == 0)
		{
			float sx = (-_b) / (2 * _a);
			float sy = (a * sx) + b;

			return std::vector<vec2> { vec2(sx, sy) };
		}
		else
		{
			float s1x = (-_b - sqrt(delta)) / (2 * _a);
			float s2x = (-_b + sqrt(delta)) / (2 * _a);

			float s1y = (a * s1x) + b;
			float s2y = (a * s2x) + b;

			return std::vector<vec2> { vec2(s1x, s1y), vec2(s2x, s2y) };
		}
	}
}

void CircleCollider::SetCollideWithPhys(bool value)
{
	if (collideWithPhys != value)
	{
		if (collideWithPhys)
			Collider::circleColliders.remove(this);
		else 
			Collider::circleColliders.push_back(this);
	}
	
	collideWithPhys = value;
}

