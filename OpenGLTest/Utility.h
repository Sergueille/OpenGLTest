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
};

