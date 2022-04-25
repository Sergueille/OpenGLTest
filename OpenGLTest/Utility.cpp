#include "Utility.h"

#include "Camera.h"

#include <iostream>
#include <string>
#include <math.h>

namespace Utility
{
	int screenX;
	int screenY;

	GLFWwindow* window = NULL;
	float time = 0;
	float lastTime = 0;

	SoLoud::Soloud* soloud = nullptr;

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
		double rad = beta * 3.14159265 / 180.0;
		return glm::vec2(
			(cos(rad) * vec.x) - (sin(rad) * vec.y),
			(sin(rad) * vec.x) + (cos(rad) * vec.y)
		);
	}

	float AngleBetween(glm::vec2 a, glm::vec2 b)
	{
		return acos(glm::dot(a, b) / glm::length(a) / glm::length(b)) * Utility::RadToDeg;
	}

	glm::vec2 GetMousePos()
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		return glm::vec2((float)mouseX, (float)mouseY);
	}

	glm::vec2 ScreenToWorld(glm::vec2 screenPos)
	{
		screenPos.y *= -1;
		screenPos.y += (float)screenY / 2;
		screenPos.x -= (float)screenX / 2;

		screenPos /= screenY;

		glm::vec2 res = Camera::position + (screenPos * Camera::size);
		return res;
	}

	glm::vec2 LineItersection(float a1, float b1, float a2, float b2)
	{
		if (a1 == a2) throw "The lines are not crossing. Please check that a1 is not equal to a2 before calling the function";

		if (std::isnan(a1))
		{
			return LineItersectionVertical(a2, b2, b1);
		}
		if (std::isnan(a2))
		{
			return LineItersectionVertical(a1, b1, b2);
		}

		float sx = (b2 - b1) / (a1 - a2);
		float sy = (a1 * sx) + b1;

		return glm::vec2(sx, sy);
	}

	glm::vec2 LineItersectionVertical(float a1, float b1, float b2)
	{
		float sy = (a1 * b2) + b1;
		float sx = b2;

		return glm::vec2(sx, sy);
	}

	void GetLineEquationFromPoints(glm::vec2 p1, glm::vec2 p2, float* a, float* b)
	{
		if (p1.x == p2.x) // Vertical
		{
			*a = NAN;
			*b = p1.x;
		}
		else // Not vertical
		{
			*a = (p2.y - p1.y) / (p2.x - p1.x);
			*b = p1.y - ((*a) * p1.x);
		}
	}

	float GetTimeSine(int period)
	{
		return (float)cos(Utility::time * Utility::PI / (period / 1000.f)) * 0.5 + 0.5; return 0.0f;
	}
}
