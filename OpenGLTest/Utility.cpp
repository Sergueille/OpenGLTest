#include "Utility.h"

#include <iostream>
#include <string>
#include <math.h>

namespace Utility
{
	GLFWwindow* window = NULL;
	float time = 0;
	float lastTime = 0;

	float GetDeltaTime()
	{
		return time - lastTime;
	}

	glm::vec3 Utility::Project(glm::vec2 a, glm::vec2 b, glm::vec2 c)
	{
		glm::vec2 AB = b - a;
		glm::vec2 AC = c - a;

		float AH = ((AB.x * AC.x) + (AB.y * AC.y)) / length(AB);
		glm::vec2 H = a + (glm::normalize(AB) * AH);

		return glm::vec3(H.x, H.y, AH);
	}

	glm::vec2 Rotate(glm::vec2 vec, float beta)
	{
		float rad = beta * 3.14159265 / 180.0;
		return glm::vec2(
			(cos(rad) * vec.x) - (sin(rad) * vec.y),
			(sin(rad) * vec.x) + (cos(rad) * vec.y)
		);
	}

	float AngleBetween(glm::vec2 a, glm::vec2 b)
	{
		return acos(glm::dot(a, b) / glm::length(a) / glm::length(b)) * Utility::RadToDeg;
	}
}
