#pragma once

#include "Sprite.h"
#include "EventManager.h"

using namespace glm;

class Particle;
class ParticleSystem
{
public:
	ParticleSystem();
	virtual ~ParticleSystem();

	vec3 emitterPosition = vec3(0);
	vec2 emitterSize = vec2(1);
	float emitterRotation = 0;
	bool emitCircle = false;

	float particleLifetime = 3;
	float particlesPerSecond = 100;
	float duration = 5;

	Sprite* paticleTemplate = nullptr;

	vec2 startVelocity = vec2(0, 0);
	vec2 endVelocity = vec2(0, 3);

	bool changeColor = false;
	vec4 startColor = vec4(1);
	vec4 endColor = vec4(1);

	bool changeSize = false;
	vec2 startSize = vec2(1);
	vec2 endSize = vec2(1);

	void Start();
	void Stop();
	bool IsPlaying();

	void SubscribeToPerticleSysEvents();

protected:
	bool isPlaying = false;
	std::list<Particle*> particles = std::list<Particle*>();

	LinkedListElement<std::function<void()>>* psysMainLoopFuncPos = nullptr;

	float lastInstantiationTime = 0;

	void OnParticleSystemMainLoop();
};

class Particle
{
public:
	Particle(Sprite* sprite);
	~Particle();

	Sprite* sprite = nullptr;
	float age = 0;
};
