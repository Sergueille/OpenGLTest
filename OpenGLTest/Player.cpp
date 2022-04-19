#include "Player.h"
#include "Camera.h"

#include <iostream>

using namespace glm;

Player::Player(vec3 position) : PhysicObject(new CircleCollider(vec2(position), height)), EditorObject(position)
{
	playerSprite = new Sprite(RessourceManager::GetTexture("Engine\\circle.png"),
		position, glm::vec2(height), 0,
		glm::vec4(1, 0, 0, 1)); // Create a sprite!
	playerSprite->DrawOnMainLoop();

	collider = new CircleCollider(position, height);
	clickCollider = collider;

	SetPos(position);

	Camera::getTarget = [this]() -> glm::vec2 { return this->GetPos(); };

	if (Editor::enabled)
	{
		this->physicsEnabled = false;
	}

	typeName = "Player";
}

Player::~Player()
{
	if (playerSprite != nullptr)
	{
		delete playerSprite;
		playerSprite = nullptr;
	}

	if (collider != nullptr)
	{
		delete collider;
		collider = nullptr;
		clickCollider = nullptr;
	}

	EventManager::OnOpenEditor.remove(subscribedFuncs[0]);
	EventManager::OnCloseEditor.remove(subscribedFuncs[1]);
}

void Player::UpdateTransform()
{
	EditorObject::UpdateTransform();
	this->SetPos(editorPosition);
	playerSprite->position = editorPosition;
}

EditorObject* Player::Copy()
{
	throw "Not implemented";
}

void Player::Enable()
{
	EditorObject::Enable();
	playerSprite->DrawOnMainLoop();
	collider->enabled = true;
	physicsEnabled = physicsWasEnabledBeforeDisabling;
}

void Player::Disable()
{
	EditorObject::Disable();
	playerSprite->StopDrawing();
	collider->enabled = false;
	physicsWasEnabledBeforeDisabling = physicsEnabled;
	physicsEnabled = false;
}

// Handle physics
void Player::OnAfterMove()
{
	playerSprite->position = vec3(GetPos().x, GetPos().y, 0);

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