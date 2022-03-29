#include "Camera.h"

#include <iostream>
#include "Utility.h"

using namespace glm;

namespace Camera
{
	const float scrollSensitivity = 0.15;
	const float scrollSmoothAmount = 15;

	glm::vec2 position = glm::vec2(0);
	std::function<glm::vec2()> getTarget = NULL;
	glm::vec2 velocity = glm::vec2(0);

	bool editorCamera = true; // TEST: set to false

	bool isDragging;
	bool wasDraggingLastFrame;
	vec2 lastMousePos;
	float sizeTarget;

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
		if (editorCamera)
		{
			// Get mouse pos
			double mouseX, mouseY;
			glfwGetCursorPos(Utility::window, &mouseX, &mouseY);

			if (isDragging)
			{
				float mult = size / Utility::screenY;
				vec2 delta = (vec2(mouseX, mouseY) - lastMousePos) * mult; // Delta
				delta.y *= -1;

				position -= delta;
			}

			// Set last mouse pos
			lastMousePos = vec2(mouseX, mouseY);

			// Update zoom
			size -= (size - sizeTarget) * Utility::GetDeltaTime() * scrollSmoothAmount;
		}
		else
		{
			// Follow target
			velocity += (getTarget() - position);
			velocity /= smoothTime;
			position += velocity * Utility::GetDeltaTime();
		}

	}

	void SetupCamera()
	{
		glfwSetMouseButtonCallback(Utility::window, OnMouseClick);
		glfwSetScrollCallback(Utility::window, OnScroll);

		sizeTarget = size;
	}

	void OnMouseClick(GLFWwindow* window, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			isDragging = action == GLFW_PRESS;
		}
	}

	void OnScroll(GLFWwindow* window, double xOffset, double yOffset)
	{
		sizeTarget -= (float)yOffset * scrollSensitivity * sizeTarget;
	}
}
