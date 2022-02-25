#include "Player.h"

#include <iostream>

Player::Player()
{
	sprite = new Sprite(&RessourceManager::textures["circle"], glm::vec3(0), glm::vec2(height)); // Create a sprite!
	EventManager::OnMainLoop.push_back([this] { this->OnMainLoop(); }); // subscribe to the main loop
}

Player::~Player()
{
	delete sprite;
}

void Player::OnMainLoop()
{
	float realSpeed = speed * Utility::GetDeltaTime();

	if (glfwGetKey(Utility::window, GLFW_KEY_W) == GLFW_PRESS)
	{
		position.y += realSpeed;
	}
	if (glfwGetKey(Utility::window, GLFW_KEY_A) == GLFW_PRESS)
	{
		position.x -= realSpeed;
	}
	if (glfwGetKey(Utility::window, GLFW_KEY_S) == GLFW_PRESS)
	{
		position.y -= realSpeed;
	}
	if (glfwGetKey(Utility::window, GLFW_KEY_D) == GLFW_PRESS)
	{
		position.x += realSpeed;
	}

	sprite->position = glm::vec3(position.x, position.y, 0);
}
