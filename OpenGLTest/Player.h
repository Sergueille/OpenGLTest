#pragma once

#include <string>
#include <glad/glad.h> 
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "PhysicObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "EventManager.h"
#include "Utility.h"
#include "CircleCollider.h"

using namespace glm;

class Player: PhysicObject
{
public:
	const float height = 1;
	/// <summary>
	/// Walk speed of the player, in units/sec
	/// </summary>
	const float walkSpeed = 6;
	/// <summary>
	/// The force applied at the beginning of the jump
	/// </summary>
	const float jumpForce = 18;
	/// <summary>
	/// The force applied when jumping and the player release spacebar
	/// </summary>
	const float jumpForceStop = 70;

	bool isJumping = false;

	Sprite* sprite = NULL;

	Player();
	~Player();

protected:
	void OnAfterMove() override;
};

