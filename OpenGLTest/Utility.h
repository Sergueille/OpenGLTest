#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <soloud.h>
#include <string>

namespace Utility
{
	extern int screenX;
	extern int screenY;

	extern GLFWwindow* window;

	extern float GetDeltaTime();
	extern float time;
	extern float lastTime;

	const float PI = 3.14159265f;
	const float HALF_PI = 3.14159265f / 2.0f;
	const float DegToRad = PI / 180.0f;
	const float RadToDeg = 180.0f / PI;

	extern SoLoud::Soloud* soloud;

	extern bool isAzerty;

	const int FPS_NB_VALUES = 150;
	extern int FPSvalues[FPS_NB_VALUES];
	extern int FPSvaluePos;

	/// <summary>
	/// Projects C on AB
	/// </summary>
	/// <returns>A vector where XY are the position of H, and Z is AH (oriented)</returns>
	extern glm::vec3 Project(glm::vec2 a, glm::vec2 b, glm::vec2 c);

	/// <summary>
	/// Rotates a vector by theta angle couter-clockwise
	/// </summary>
	extern glm::vec2 Rotate(glm::vec2 vec, float beta);

	extern float AngleBetween(glm::vec2 a, glm::vec2 b);

	/// <summary>
	/// Get angle of the vector from right, counter clockwise
	/// </summary>
	extern float GetVectorAngle(glm::vec2 vector);

	/// <summary>
	/// Get mouse pos in screen coordinates (top-left is 0)
	/// </summary>
	extern glm::vec2 GetMousePos();

	/// <summary>
	/// Get world position from screen position (raw pixels, not 0-1)
	/// </summary>
	extern glm::vec2 ScreenToWorld(glm::vec2 screenPos);

	/// <summary>
	/// <para> Get the intersection of two lines of equation x = a1 y + b1 and x = a2 y + b2 </para>
	/// <para> Check that the lines are crossing before calling the function with a1 != a2 </para>
	/// <para> However, it handles the vertical line case with a = nan </para>
	/// </summary>
	extern glm::vec2 LineItersection(float a1, float b1, float a2, float b2);

	/// <summary>
	/// Same as LineItersection(), but the second equation is x = b2
	/// </summary>
	extern glm::vec2 LineItersectionVertical(float a1, float b1, float b2);

	/// <summary>
	/// Get the intersection of the segment A1 B1 and A2 B2, if none, don't do anything
	/// </summary>
	extern bool SegementIntersection(glm::vec2 A1, glm::vec2 B1, glm::vec2 A2, glm::vec2 B2, glm::vec2* res);

	/// <summary>
	/// <br>Get a line equation that touch p1 and p2</br>
	/// <br>If the line is vertical, it will set a to nan, check with std::isnan() in math.h</br>
	/// </summary>
	extern void GetLineEquationFromPoints(glm::vec2 p1, glm::vec2 p2, float* a, float* b);

	/// <summary>
	/// Get the cos of the timen rescaled btw 0 and 1
	/// </summary>
	/// <param name="period">The duration of one sine (milliseconds)</param>
	extern float GetTimeSine(int period);

	int GetFPS();

	extern std::string GetKeyDesc(int glfwKey);

	extern std::string ToLower(std::string value);
	extern std::string ToUpper(std::string value);
};

