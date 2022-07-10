#include "Player.h"
#include "Camera.h"
#include "Laser.h"
#include "CircleCollider.h"
#include "TweenManager.h"

#include <iostream>
#include <soloud_lofifilter.h>
#include "ParticleSystem.h"

using namespace glm;

ObjectEvent Player::events[PLAYER_EVENT_COUNT] = {
	ObjectEvent {
		"Kill",
		[](EditorObject* object, void* param) { ((Player*)object)->Kill(); },
	},
	ObjectEvent {
		"Turn on",
		[](EditorObject* object, void* param) { ((Player*)object)->isOn = true; },
	},
	ObjectEvent {
		"Give teleportation",
		[](EditorObject* object, void* param) { ((Player*)object)->GiveTeleportation(); },
	},
	ObjectEvent {
		"Turn off",
		[](EditorObject* object, void* param) { ((Player*)object)->isOn = false; },
	},
	ObjectEvent {
		"Remove teleportation",
		[](EditorObject* object, void* param) { ((Player*)object)->RemoveTeleportation(); },
	},
};

Player* Player::ingameInstance = nullptr;

const float Player::levSoundMinVolume = 0.3f;
const float Player::levSoundMaxVolume = 0.15f;
const float Player::levSoundMinPitch = 0.8f;
const float Player::levSoundMaxPitch = 0.4f;
const float Player::levSoundMinDist = 0.5f;
const float Player::levSoundMaxDist = 4.0f;
const float Player::levSoundSmoothAmount = 0.2f; // In seconds

Player::Player(vec3 position) : PhysicObject(new CircleCollider(vec2(position), height, true)), EditorObject(position)
{
	playerSprite = new Sprite(RessourceManager::GetTexture("robot.png"),
		position, glm::vec2(height), 0,
		glm::vec4(1.5, 1.5, 1.5, 1)); // Create a sprite!
	playerSprite->isLit = true;
	playerSprite->DrawOnMainLoop();

	teleportPosSprite = new Sprite(RessourceManager::GetTexture("robot.png"),
		position + vec3(teleportationDistance, 0, 0),
		glm::vec2(height), 0,
		canTeleportColor,
		&RessourceManager::shaders["teleport"]);
	teleportPosSprite->DrawOnMainLoop();

	lightsSprite = new Sprite(RessourceManager::GetTexture("robot_lights.png"),
		position, glm::vec2(height * 0.23f), 0,
		normalLightColor); // Create a sprite!
	lightsSprite->DrawOnMainLoop();

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

		// Play levitation sound
		levitationSoundHandle = PlaySound(levitationSound, 0);
		soloud->setLooping(levitationSoundHandle, true);
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

	if (lightsSprite != nullptr)
	{
		delete lightsSprite;
		lightsSprite = nullptr;
	}

	if (collider != nullptr)
	{
		delete collider;
		clickCollider = nullptr;
		collider = nullptr;
	}

	if (ingameInstance == this)
		ingameInstance = nullptr;

	if (!Editor::enabled)
	{
		soloud->stop(levitationSoundHandle);
	}
}

void Player::OnMainLoop()
{
	EditorObject::OnMainLoop();

	if (Editor::enabled && enabled)
	{
		this->SetPos(editorPosition);
		playerSprite->position = editorPosition;
		if (teleportPosSprite != nullptr)
			teleportPosSprite->position = editorPosition + vec3(teleportationDistance, 0, 0);
		lightsSprite->position = editorPosition + height * vec3(0, -0.344, 1);
	}
	else if (enabled)
	{
		collider->SetPos(GetPos()); // Re-set collider position because EditorObject will replace it by default

		float deltaY = isOn ? GetTimeSine((int)(floatPeriod * 1000.0f)) * floatIntensity - (floatIntensity / 2) : 0;

		// Set sprite position
		playerSprite->position = vec3(GetPos().x, GetPos().y + deltaY, editorPosition.z);
		vec2 lightPos = GetPos() + height * vec2(0, -0.344);;
		lightsSprite->position = vec3(lightPos.x, lightPos.y + deltaY, editorPosition.z + 1);
	}
}

vec2 Player::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::CheckBox(drawPos, "Can teleport", &canTeleport, Editor::panelPropertiesX).y;
	drawPos.y -= Editor::CheckBox(drawPos, "Start on", &isOn, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* Player::Copy()
{
	Player* copy = new Player(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	copy->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());
	copy->collider = copy->clickCollider;

	// Copy sprites
	copy->playerSprite = this->playerSprite->Copy();
	if (this->teleportPosSprite != nullptr)
		copy->teleportPosSprite = this->teleportPosSprite->Copy(); 
	copy->lightsSprite = this->lightsSprite->Copy();

	copy->SubscribeToMainLoop();

	return copy;
}

void Player::Enable()
{
	EditorObject::Enable();
	playerSprite->DrawOnMainLoop();
	if (teleportPosSprite != nullptr) 
		teleportPosSprite->DrawOnMainLoop();
	lightsSprite->DrawOnMainLoop();
	collider->enabled = true;
	physicsEnabled = physicsWasEnabledBeforeDisabling;
}

void Player::Disable()
{
	EditorObject::Disable();
	playerSprite->StopDrawing();
	if (teleportPosSprite != nullptr) 
		teleportPosSprite->StopDrawing();
	lightsSprite->StopDrawing();
	collider->enabled = false;
	physicsWasEnabledBeforeDisabling = physicsEnabled;
	physicsEnabled = false;
}

void Player::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("canTeleport", canTeleport);
	EditorSaveManager::WriteProp("isOn", isOn);
}

void Player::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);
	SetPos(editorPosition);
	canTeleport = (*props)["canTeleport"] != "0";
	isOn = (*props)["isOn"] != "0";

	// Don't need the sprite if no teleportation allowed
	if (!Editor::enabled && !canTeleport)
	{
		teleportPosSprite->StopDrawing();
	}

	if (!Editor::enabled)
	{
		Camera::position = GetEditPos();
	}
}

void Player::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = events;
	*resCount = PLAYER_EVENT_COUNT;
}

void Player::Kill()
{
	if (isDying) return;

	isDying = true;
	physicsEnabled = false;

	TweenManager<float>::Tween(Camera::size, deathZoom, deathDuration, [this](float value) {
		Camera::SetSize(value);
	}, sineIn);

	TweenManager<float>::Tween(Camera::position.y, Camera::position.y - deathCameraShift, deathDuration, [this](float value) {
		Camera::position.y = value;
	}, sineIn);

	TweenManager<float>::Tween(0, 1, deathDuration, [this](float value) {
		Utility::corruptionAmount = value;
	}, cubicOut)->SetOnFinished([] {
		Utility::corruptionAmount = 0;
	});

	TweenManager<float>::Tween(0, 1, deathDuration, [](float value) {
		Utility::soloud->setFilterParameter(0, 0, SoLoud::LofiFilter::WET, value);
	}, linear)->SetOnFinished([] {
		Utility::soloud->setFilterParameter(0, 0, SoLoud::LofiFilter::WET, 0);
	});

	TweenManager<vec2>::Tween(GetPos(), vec2(GetPos()) - vec2(0, deathPlayerShift), deathDuration, [this](vec2 value) {
		SetPos(vec2(value.x, value.y));
	}, linear)->SetOnFinished([this] {
		EditorSaveManager::LoadUserSaveInSameLevel(EditorSaveManager::currentUserSave);
		isDying = false;
		physicsEnabled = true;
	});
}

// Handle physics
void Player::OnAfterMove()
{
	if (!isOn)
	{
		soloud->setVolume(levitationSoundHandle, 0);
		this->lightsSprite->color = offLightColor;
		return;
	}

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

	bool isClicking = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	if (canTeleport)
	{
		// Get teleport position
		vec2 worldMousePos = ScreenToWorld(GetMousePos());
		vec2 mouseDirection = glm::normalize(worldMousePos - GetPos());
		teleportPosition = GetPos() + mouseDirection * teleportationDistance;
		teleportPosSprite->position = vec3(teleportPosition.x, teleportPosition.y, editorPosition.z);

		// Determines if teleportation not in a wall
		CircleCollider teleportCollider = CircleCollider(teleportPosition, height, false);
		bool isColliding = teleportCollider.IsTouchingAnyCollider();
		bool hitLaser = TeleportCollideWithLaser(teleportPosition);
		bool canTeleprt = teleportationsRemaining > 0 && !isColliding && !hitLaser;

		// Set sprite color
		teleportPosSprite->color = canTeleprt ? canTeleportColor : cannotTeleportColor;

		if (isClicking && !wasClickingLastFrame) // Click !!!
		{
			if (canTeleprt)
			{
				SetPos(teleportPosition);
				velocity.y += teleportVerticalForce;
				isJumping = false;

				TeleportEffect();

				teleportationsRemaining--;
			}
			else
			{
				teleportPosSprite->color = cannotTeleportClickColor;
			}
		}
	}

	wasClickingLastFrame = isClicking;

	// Ground raycast
	vec2 res1; vec2 res2;
	vec2 origin1 = GetPos() + vec2(groudRaycastsXshift, 0);
	vec2 origin2 = GetPos() + vec2(-groudRaycastsXshift, 0);

	Collider* hitColl1 = nullptr;
	Collider* hitColl2 = nullptr;

	bool found1 = Collider::Raycast(origin1, vec2(0, -1), &res1, collider, &hitColl1);
	bool found2 = Collider::Raycast(origin2, vec2(0, -1), &res2, collider, &hitColl2);
	if (found2 || found1)
	{
		float dist1 = glm::length(res1 - origin1);
		float dist2 = glm::length(res2 - origin2);

		float dist = min(dist1, dist2);
		Collider* hitColl = dist1 < dist2 ? hitColl1 : hitColl2;

		// Update sound
		float amount;
		if (dist <= levSoundMinDist) amount = 0;
		else amount = (dist - levSoundMinDist) / (levSoundMaxDist - levSoundMinDist);
		if (amount > 1) amount = 1;

		// Smooth to avoid brutal changes
		levSoundDistortAmount = levSoundDistortAmount + ((amount - levSoundDistortAmount) * GetDeltaTime() / levSoundSmoothAmount);

		float volume = Utility::Lerp(levSoundMinVolume, levSoundMaxVolume, levSoundDistortAmount);
		float pitch = Utility::Lerp(levSoundMinPitch, levSoundMaxPitch, levSoundDistortAmount);
		soloud->setVolume(levitationSoundHandle, volume * gameSoundsVolume);
		soloud->setRelativePlaySpeed(levitationSoundHandle, pitch);

		if (dist < floatingForceStartDist && (!isJumping || velocity.y < 0)) // Levitation
		{
			float quantity;
			if (dist > floatingDistance)
				quantity = 1 - ((dist - floatingDistance) / floatingForceStartDist);
			else
				quantity = ((1 - (dist / floatingDistance)) * (maxFloatForceMultiplicator - 1)) + 1;

			float force = (gravityAcceleration - (velocity.y / GetDeltaTime() * floatForceFactor)) * quantity;
			velocity.y += force * GetDeltaTime();

			this->lightsSprite->color = (quantity / maxFloatForceMultiplicator) * (brightLightColor - normalLightColor) + normalLightColor;

			if (hitColl != nullptr)
			{
				SetPos(GetPos() + hitColl->deltaPos); // Move with the collider;
			}
		}
		else
		{
			this->lightsSprite->color = normalLightColor;
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

	SetEditPos(vec3(GetPos().x, GetPos().y, GetEditPos().z)); // TODO: can be optimised
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
		if ((*it)->laserType == Laser::LaserType::noTeleport && (*it)->IsEnabled() && (*it)->laserOn) 
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

void Player::TeleportEffect()
{
	Sprite* pulse = new Sprite(
		RessourceManager::GetTexture("pulse.png"),
		vec3(GetPos().x, GetPos().y, GetEditPos().z - 1), vec2(0.1f), 0, vec4(1.1f, 0.3f, 0.3f, 1)
	);
	pulse->DrawOnMainLoop();

	ParticleSystem* psys = new ParticleSystem();
	psys->duration = 0.2f;
	psys->particlesPerSecond = 200.0f;
	psys->particleLifetime = 1.0f;
	psys->changeSize = true;
	psys->startSize = vec2(0.2f);
	psys->endSize = vec2(0);
	psys->changeColor = true;
	psys->startColor = vec4(1, 0.2f, 0.2f, 0.9f);
	psys->endColor = vec4(1, 0.7f, 0.7f, 0.5f);
	psys->velocityFromAngle = true;
	psys->startVelocity = vec2(0, 6);
	psys->endVelocity = vec2(0, 0);
	psys->velocityFromAngle = true;
	psys->emitterPosition = vec3(GetPos().x, GetPos().y, 60);
	psys->emitterSize = vec2(height);
	psys->emitCircle = true;
	psys->deleteWhenStopped = true;
	psys->paticleTemplate = new Sprite(RessourceManager::GetTexture("Engine\\circle.png"), vec3(0), vec2(1), 0);
	psys->Start();

	TweenManager<float>::Tween(0, 1, 0.5f, [pulse](float value) {
		float size = Utility::Lerp<float>(0.1f, 8, value);
		float alpha = 1 - value;

		pulse->size = vec2(size);
		pulse->color.a = alpha;
	}, sineOut)->SetOnFinished([pulse] {
		delete pulse;
	});
}

void Player::GiveTeleportation()
{
	teleportPosSprite->DrawOnMainLoop();
	canTeleport = true;
}

void Player::RemoveTeleportation()
{
	teleportPosSprite->StopDrawing();
	canTeleport = false;
}
