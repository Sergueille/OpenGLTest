#include "RectCollider.h"
#include "Collider.h"
#include "Utility.h"

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
