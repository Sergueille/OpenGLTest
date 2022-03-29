#include "Collider.h"

std::list<CircleCollider*> Collider::circleColliders = std::list<CircleCollider*>();
std::list<RectCollider*> Collider::rectColliders = std::list<RectCollider*>();