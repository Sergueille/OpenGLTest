#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

namespace Utility
{
	static const int screenX = 1280;
	static const int screenY = 720;

	extern GLFWwindow* window;

	extern float GetDeltaTime();
	extern float time;
	extern float lastTime;

	const float PI = 3.14159265f;
	const float HALF_PI = 3.14159265f / 2.0f;
	const float DegToRad = PI / 180.0f;
	const float RadToDeg = 180.0f / PI;

	/// <summary>
	/// Projects C on AB
	/// </summary>
	/// <returns>A vector where XY are the position of H, and Z is AH (oriented)</returns>
	extern glm::vec3 Project(glm::vec2 a, glm::vec2 b, glm::vec2 c);

	extern glm::vec2 Rotate(glm::vec2 vec, float beta);

	extern float AngleBetween(glm::vec2 a, glm::vec2 b);

	/// <summary>
	/// Get mouse pos in screen coordinates (top-left is 0)
	/// </summary>
	extern glm::vec2 GetMousePos();

	/// <summary>
	/// Get world position from screen position (raw pixels, not 0-1)
	/// </summary>
	extern glm::vec2 ScreenToWorld(glm::vec2 screenPos);

	/// <summary>
	/// Get the cos of the timen rescaled btw 0 and 1
	/// </summary>
	/// <param name="period">The duration of one sine (milliseconds)</param>
	extern float GetTimeSine(int period);
};

