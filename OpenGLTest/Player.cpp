#include "Player.h"
#include "Camera.h"

#include <iostream>

using namespace glm;

Player::Player() : PhysicObject(new CircleCollider(GetPos(), height))
{
	sprite = new Sprite(&RessourceManager::textures["circle"], 
		glm::vec3(0), glm::vec2(height), 0,
		glm::vec4(1, 0, 0, 1)); // Create a sprite!

	collider = new CircleCollider(GetPos(), height);

	Camera::getTarget = [this]() -> glm::vec2 { return this->GetPos(); };
}

Player::~Player()
{
	delete sprite;
	delete collider;
}

void Player::OnAfterMove()
{
	sprite->position = vec3(GetPos().x, GetPos().y, 0);

	float realSpeed = walkSpeed;

	if (glfwGetKey(Utility::window, GLFW_KEY_A) == GLFW_PRESS)
	{
		velocity.x = -realSpeed;
	}
	else if (glfwGetKey(Utility::window, GLFW_KEY_D) == GLFW_PRESS)
	{
		velocity.x = realSpeed;
	}
	else
	{
		velocity.x = 0;
	}

	// Jump !
	if (isOnWalkableSurface) 
	{
		isJumping = false;
		if (glfwGetKey(Utility::window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(Utility::window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			velocity.y = jumpForce;
			isJumping = true;
		}
	}
	else
	{
		if (isJumping && velocity.y > 0)
		{
			if (glfwGetKey(Utility::window, GLFW_KEY_W) != GLFW_PRESS && glfwGetKey(Utility::window, GLFW_KEY_SPACE) != GLFW_PRESS)
			{
				velocity.y -= jumpForceStop * Utility::GetDeltaTime();
			}
		}
	}
}