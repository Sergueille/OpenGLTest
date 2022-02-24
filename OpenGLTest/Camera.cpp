#include "Camera.h"

#include "Utility.h"

glm::mat4 Camera::GetOrthographicProjection()
{
	float halfSize = size / 2;
	float XhalfSize = (halfSize / Utility::screenY) * Utility::screenX;
	return glm::ortho(position.x - XhalfSize, position.x + XhalfSize, position.y - halfSize, position.y + halfSize, -1.0f, 100.0f);
}
