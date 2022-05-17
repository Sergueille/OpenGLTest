#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>

namespace Camera
{
	extern const float defaultSize;

	extern glm::vec2 position;
	extern std::function<glm::vec2()> getTarget;
	extern glm::vec2 velocity;

	/// <summary>
	/// The height of the wiew in world units.
	/// Width is determined by ratio.
	/// </summary>
	extern float size;
	extern float smoothTime;

	extern bool editorCamera;

	/// <summary>
	/// Pass this into vertex shaders for the projection matrix
	/// </summary>
	glm::mat4 GetOrthographicProjection();

	/// <summary>
	/// Pass this into vertex shaders for the projection matrix
	/// </summary>
	glm::mat4 GetUIProjection();

	void SetupCamera();

	void UpdateCamera();

	void OnMouseClick(GLFWwindow* window, int button, int action, int mods);

	void OnScroll(GLFWwindow* window, double xOffset, double yOffset);

	void SetSize(float value);
}

