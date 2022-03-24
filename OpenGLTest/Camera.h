#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>

namespace Camera
{
	extern glm::vec2 position;
	extern std::function<glm::vec2()> getTarget;
	extern glm::vec2 velocity;

	/// <summary>
	/// The height of the wiew in world units.
	/// Width is determined by ratio.
	/// </summary>
	extern float size;
	extern float smoothTime;

	/// <summary>
	/// Pass this into vertex shaders for the projection matrix
	/// </summary>
	glm::mat4 GetOrthographicProjection();

	void UpdateCamera();
}

