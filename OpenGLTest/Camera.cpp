#include "Camera.h"

#include "Utility.h"

namespace Camera
{
	glm::vec2 position = glm::vec2(0);
	std::function<glm::vec2()> getTarget = NULL;
	glm::vec2 velocity = glm::vec2(0);

	float size = 16;
	float smoothTime = 1.3f;

	glm::mat4 GetOrthographicProjection()
	{
		float halfSize = size / 2;
		float XhalfSize = (halfSize / Utility::screenY) * Utility::screenX;
		return glm::ortho(position.x - XhalfSize, position.x + XhalfSize, position.y - halfSize, position.y + halfSize, -1.0f, 100.0f);
	}

	void UpdateCamera()
	{
		velocity += (getTarget() - position);
		velocity /= smoothTime;
		position += velocity * Utility::GetDeltaTime();
	}
}
