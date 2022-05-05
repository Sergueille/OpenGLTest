#include "Player.h"
#include "Camera.h"
#include "Laser.h"
#include "CircleCollider.h"

#include <iostream>

using namespace glm;

Player* Player::ingameInstance = nullptr;

Player::Player(vec3 position) : PhysicObject(new CircleCollider(vec2(position), height, true)), EditorObject(position)
{
	playerSprite = new Sprite(RessourceManager::GetTexture("Engine\\circle.png"),
		position, glm::vec2(height), 0,
		glm::vec4(1, 0, 0, 1)); // Create a sprite!
	playerSprite->DrawOnMainLoop();

	teleportPosSprite = new Sprite(RessourceManager::GetTexture("Engine\\circle.png"),
		position + vec3(teleportationDistance, 0, 0),
		glm::vec2(height), 0,
		canTeleportColor);
	teleportPosSprite->DrawOnMainLoop();

	((CircleCollider*)collider)->size = height;
	clickCollider = collider;

	SetPos(position);

	Camera::getTarget = [this]() -> glm::vec2 { return this->GetPos(); };

	if (Editor::enabled)
	{
		this->physicsEnabled = false;
	}
	else
	{
		if (ingameInstance == nullptr)
			ingameInstance = this;
		else
			std::cout << "Two player instaces in game !!" << std::endl;
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

	if (teleportPosSprite != nullptr)
	{
		delete teleportPosSprite;
		teleportPosSprite = nullptr;
	}

	if (collider != nullptr)
	{
		delete collider;
		clickCollider = nullptr;
		collider = nullptr;
	}

	ingameInstance = nullptr;
}

void Player::UpdateTransform()
{
	EditorObject::UpdateTransform();
	this->SetPos(editorPosition);
	playerSprite->position = editorPosition;
	teleportPosSprite->position = editorPosition + vec3(teleportationDistance, 0, 0);
}

EditorObject* Player::Copy()
{
	Player* copy = new Player(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	copy->clickCollider = new CircleCollider(oldCollider->position, oldCollider->size, oldCollider->MustCollideWithPhys());
	copy->collider = copy->clickCollider;

	// Copy sprites
	copy->playerSprite = this->playerSprite->Copy();
	copy->teleportPosSprite = this->teleportPosSprite->Copy(); 

	copy->SubscribeToMainLoop();
	copy->SubscribeToEditorObjectFuncs();

	return copy;
}

void Player::Enable()
{
	EditorObject::Enable();
	playerSprite->DrawOnMainLoop();
	teleportPosSprite->DrawOnMainLoop();
	collider->enabled = true;
	physicsEnabled = physicsWasEnabledBeforeDisabling;
}

void Player::Disable()
{
	EditorObject::Disable();
	playerSprite->StopDrawing();
	teleportPosSprite->StopDrawing();
	collider->enabled = false;
	physicsWasEnabledBeforeDisabling = physicsEnabled;
	physicsEnabled = false;
}

// Handle physics
void Player::OnAfterMove()
{
	// Set sprite position
	playerSprite->position = vec3(GetPos().x, GetPos().y, editorPosition.z);

	float realSpeed = walkSpeed;

	// Hanlde horizontal movement
	if (glfwGetKey(Utility::window, GLFW_KEY_A) == GLFW_PRESS)
	{
		float deltaVelocity = (-realSpeed) - velocity.x; // Difference of velocity to reach target velocity
		float targetForce = deltaVelocity / GetDeltaTime(); // Get force to apply to reach target velocity
		if (targetForce < -walkMaxForce) targetForce = -walkMaxForce; // Clamp force value
		velocity.x += targetForce * GetDeltaTime(); // Apply force
	}
	else if (glfwGetKey(Utility::window, GLFW_KEY_D) == GLFW_PRESS)
	{
		float deltaVelocity = realSpeed - velocity.x; // Difference of velocity to reach target velocity
		float targetForce = deltaVelocity / GetDeltaTime(); // Get force to apply to reach target velocity
		if (targetForce > walkMaxForce) targetForce = walkMaxForce; // Clamp force value
		velocity.x += targetForce * GetDeltaTime(); // Apply force
	}
	else
	{
		float deltaVelocity = 0 - velocity.x; // Difference of velocity to reach target velocity
		float targetForce = deltaVelocity / GetDeltaTime(); // Get force to apply to reach target velocity
		if (targetForce > walkMaxForce) targetForce = walkMaxForce; // Clamp force value
		if (targetForce < -walkMaxForce) targetForce = -walkMaxForce; // Clamp force value
		velocity.x += targetForce * GetDeltaTime(); // Apply force
	}

	// Get teleport position
	vec2 worldMousePos = ScreenToWorld(GetMousePos());
	vec2 mouseDirection = glm::normalize(worldMousePos - GetPos());
	vec2 teleportPos = GetPos() + mouseDirection * teleportationDistance;
	teleportPosSprite->position = vec3(teleportPos.x, teleportPos.y, editorPosition.z);

	// Determines if teleportation not in a wall
	CircleCollider teleportCollider = CircleCollider(teleportPos, height, false);
	bool isColliding = teleportCollider.IsTouchingAnyCollider();
	bool hitLaser = TeleportCollideWithLaser(teleportPos);
	bool canTeleprt = teleportationsRemaining > 0 && !isColliding && !hitLaser;

	// Set sprite color
	teleportPosSprite->color = canTeleprt ? canTeleportColor : cannotTeleportColor;

	bool isClicking = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	if (isClicking && !wasClickingLastFrame) // Click !!!
	{
		if (canTeleprt)
		{
			SetPos(teleportPos);
			velocity.y += teleportVerticalForce;
			isJumping = false;

			teleportationsRemaining--;
		}
		else
		{
			teleportPosSprite->color = cannotTeleportClickColor;
		}
	}

	wasClickingLastFrame = isClicking;

	// Ground raycast
	vec2 res1; vec2 res2;
	vec2 origin1 = GetPos() + vec2(groudRaycastsXshift, 0);
	vec2 origin2 = GetPos() + vec2(-groudRaycastsXshift, 0);

	bool found1 = Collider::Raycast(origin1, vec2(0, -1), &res1, collider);
	bool found2 = Collider::Raycast(origin2, vec2(0, -1), &res2, collider);
	if (found2 || found1)
	{
		float dist1 = glm::length(res1 - origin1);
		float dist2 = glm::length(res2 - origin2);

		float dist = min(dist1, dist2);

		if (dist < floatingForceStartDist && (!isJumping || velocity.y < 0)) // Levitation
		{
			float maxForce = PhysicObject::gravityAcceleration * ((1 / (floatingForceStartDist - floatingDistance)) + 1);
			float force = (1 - (dist / floatingForceStartDist)) * maxForce;
			velocity.y += force * GetDeltaTime();
			
			float frictionForce = floatFriction * (-velocity.y);
			velocity.y += frictionForce * GetDeltaTime();
		}

		// Jump !
		if (dist < floatingDistanceToJump)
		{
			if (dist < floatingDistance)
				isJumping = false;

			teleportationsRemaining = maxTeleprtationInAir;

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
}

bool Player::TeleportCollideWithLaser(vec2 teleportPosition)
{
	vec2 middle = (teleportPosition + GetPos()) / 2.f;
	float length = glm::length(teleportPosition - GetPos());
	float angle = GetVectorAngle(teleportPosition - GetPos());

	RectCollider teleportCollider = RectCollider(middle, vec2(length, this->height), angle, false);
	CircleCollider teleportEndCollider = CircleCollider(teleportPosition, this->height, false);

	for (auto it = Laser::lasers.begin(); it != Laser::lasers.end(); it++)
	{
		if ((*it)->laserType == Laser::LaserType::noTeleport || (*it)->laserType == Laser::LaserType::disableTeleport)
		{
			bool rectColl = teleportCollider.CollideWith((*it)->laserCollider).z != 0;
			if (rectColl)
				return true;

			bool circleColl = teleportEndCollider.CollideWith((*it)->laserCollider).z != 0;
			if (circleColl)
				return true;

			bool playerColl = ((CircleCollider*)collider)->CollideWith((*it)->laserCollider).z != 0;
			if (playerColl)
				return true;
		}
	}

	return false;
}
