#include "ParticleSystem.h"

#include <iostream>
#include <random>
#include "Camera.h"

ParticleSystem::ParticleSystem()
{
	SubscribeToPerticleSysEvents();
}

ParticleSystem::~ParticleSystem()
{
	if (IsPlaying())
	{
		Stop();
	}

	if (psysMainLoopFuncPos != nullptr)
	{
		EventManager::OnMainLoop.remove(psysMainLoopFuncPos);

		psysMainLoopFuncPos = nullptr;
	}

	if (paticleTemplate != nullptr)
	{
		delete paticleTemplate;
		paticleTemplate = nullptr;
	}
}

void ParticleSystem::Start()
{
	isPlaying = true;
	lastInstantiationTime = Utility::time;

	if (duration > 0)
	{
		endTime = Utility::time + duration;
	}
}

void ParticleSystem::Stop()
{
	if (!IsPlaying()) return;
	isPlaying = false;

	for (auto particle = particles.begin(); particle != particles.end(); particle++)
	{
		delete (*particle);
	}
	particles.clear();

	if (deleteWhenStopped)
	{
		// Force remove evet on next frame to avoid corrupting event list
		auto localPsysMainLoopFuncPos = psysMainLoopFuncPos;
		EventManager::DoInOneFrame([localPsysMainLoopFuncPos] { EventManager::OnMainLoop.remove(localPsysMainLoopFuncPos); });
		psysMainLoopFuncPos = nullptr;

		delete this;
	}
}

bool ParticleSystem::IsPlaying()
{
	return isPlaying;
}

void ParticleSystem::SubscribeToPerticleSysEvents()
{
	psysMainLoopFuncPos = EventManager::OnMainLoop.push_end([this] { this->OnParticleSystemMainLoop(); });
}

void ParticleSystem::OnParticleSystemMainLoop()
{
	if (!IsPlaying()) return;

	if (paticleTemplate == nullptr)
	{
		std::cout << "Particle system does not have sprite template!" << std::endl;
		return;
	}

	// Wait for the last particles to be destroyed
	if (duration > 0 && endTime + particleLifetime < Utility::time)
	{
		Stop();
		return;
	}

	// Pause if far from camera
	float camDist = glm::length(Camera::position - vec2(emitterPosition));
	float maxDist = (Camera::size * screenX / screenY) + cameraMinDist;
	if (camDist > maxDist)
	{
		lastInstantiationTime = Utility::time; // Prevent creating too many particles
		return;
	}

	// Add new particles
	float deltaTime = Utility::time - lastInstantiationTime;
	if (deltaTime > 1) deltaTime = 1; // Prevent too many particules instantiation in case of FPS drop

	if (duration < 0 || endTime > Utility::time) // Stop emitting at end
	{
		int newParticlesCount = (int)(particlesPerSecond * deltaTime);
		for (int i = 0; i < newParticlesCount; i++)
		{
			Sprite* copy = paticleTemplate->Copy();
			Particle* newPart = new Particle(copy);

			const int precision = 10000;

			if (emitCircle)
			{
				float randomDist = ((float)(rand() % precision) / precision); // Between 0 and 1
				float randomAngle = ((float)(rand() % precision) / precision) * 360; // Between 0 and 360

				copy->position = emitterPosition + vec3(
					randomDist * cos(randomAngle) * emitterSize.x / 2,
					randomDist * sin(randomAngle) * emitterSize.y / 2,
					0
				);
			}
			else
			{
				float randomX = ((float)(rand() % precision) / precision) * 2 - 1; // Between -1 and 1
				float randomY = ((float)(rand() % precision) / precision) * 2 - 1; // Between -1 and 1
				vec2 vecX = Rotate(vec2(emitterSize.x / 2, 0), emitterRotation) * randomX;
				vec2 vecY = Rotate(vec2(0, emitterSize.y / 2), emitterRotation) * randomY;
				copy->position = emitterPosition + vec3(vecX.x + vecY.x, vecX.y + vecY.y, 0);
			}

			if (velocityFromAngle)
			{
				newPart->yDirection = vec2(copy->position) - vec2(emitterPosition);
				newPart->xDirection = Utility::Rotate(newPart->yDirection, -90);
			}

			particles.push_back(newPart);
			lastInstantiationTime = Utility::time;
		}
	}
	
	float deltaZ = 0; // Prevent Z fighting

	// Update particles
	for (auto it = particles.begin(); it != particles.end(); it++)
	{
		Particle* particle = (*it);

		// Destoy if too old
		if (particle->age > particleLifetime)
		{
			delete particle;
			it = particles.erase(it);

			if (it == particles.end())
				break;

			continue;
		}

		// Change & apply velocity
		vec2 velocity = Lerp<vec2>(startVelocity, endVelocity, particle->age / particleLifetime);
		vec2 delta = (particle->xDirection * velocity.x + particle->yDirection * velocity.y) * GetDeltaTime();
		particle->sprite->position += vec3(delta.x, delta.y, 0);
		particle->sprite->position.z = emitterPosition.z + deltaZ;

		if (changeColor)
			particle->sprite->color = Lerp<vec4>(startColor, endColor, particle->age / particleLifetime);

		if (changeSize)
			particle->sprite->size = Lerp<vec2>(startSize, endSize, particle->age / particleLifetime);

		// DrawSprite
		particle->sprite->Draw();

		particle->age += GetDeltaTime(); // Increment particle age

		deltaZ += 0.01f;
	}
}

Particle::Particle(Sprite* sprite)
{
	this->sprite = sprite;
}

Particle::~Particle()
{
	if (sprite != nullptr)
	{
		delete sprite;
		sprite = nullptr;
	}
}
