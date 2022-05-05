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
#include "EditorObject.h"

using namespace glm;

class Player: public PhysicObject, public EditorObject
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
	/// <summary>
	/// The max force the player can apply to try to reach his walk speed, in newtons
	/// </summary>
	const float walkMaxForce = 60;
	const float teleportationDistance = 4.5f;
	const int maxTeleprtationInAir = 2;

	/// <summary>
	/// When teleporting, add this to the y velocity
	/// </summary>
	const float teleportVerticalForce = 3;

	/// <summary>
	/// The distance btw the round and the player while floating
	/// </summary>
	const float floatingDistance = 0.55f;
	/// <summary>
	/// The min distance to jump and reload
	/// </summary>
	const float floatingDistanceToJump = 1.2f;
	/// <summary>
	/// The min distance to apply resistance force
	/// </summary>
	const float floatingForceStartDist = 2.f;
	/// <summary>
	/// Force opposed to the floating force each second
	/// </summary>
	const float floatFriction = 0.8f;

	const float groudRaycastsXshift = 0.4f;

	const vec4 canTeleportColor = vec4(0, 1, 0, 0.5);
	const vec4 cannotTeleportColor = vec4(1, 0, 0, 0.5);
	const vec4 cannotTeleportClickColor = vec4(0.8, 0.5, 0, 0.5);

	static Player* ingameInstance;

	bool isJumping = false;

	int teleportationsRemaining = maxTeleprtationInAir;

	Player(vec3 position);
	~Player();

	virtual void UpdateTransform() override;
	virtual EditorObject* Copy() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void Load(std::map<std::string, std::string>* props);

protected:
	void OnAfterMove() override;

private:
	//LinkedListElement<std::function<void()>>* subscribedFuncs[2];

	Sprite* playerSprite;
	Sprite* teleportPosSprite;

	bool physicsWasEnabledBeforeDisabling = false;
	bool wasClickingLastFrame = false;

	bool TeleportCollideWithLaser(vec2 teleportPosition);
};

