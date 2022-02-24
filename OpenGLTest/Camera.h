#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Camera
{
	/// <summary>
	/// The position of the camera
	/// </summary>
	static glm::vec2 position = glm::vec2(0);

	/// <summary>
	/// The height of the wiew in world units.
	/// Width is determined by ratio.
	/// </summary>
	static float size = 10;

	/// <summary>
	/// Pass this into vertex shaders for the projection matrix
	/// </summary>
	glm::mat4 GetOrthographicProjection();
}

