#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class ObjectFollower : public EditorObject
{
public:
	ObjectFollower();
	~ObjectFollower();

	int targetID = -1;
	float acceleration = 1; // u.s-2
	float maxSpeed = 2; // u.s-1
	float drag = 0.8f; // u.s-2
	float targetRadius = 0.2f; // If distance to target is less than that, stop acceleration

	bool rotateTowardsTarget = true;
	float rotateSpeed = 180; // deg.s-1

	bool collideWithPhysics = false;
	float bounce = 0.3f; // force is multiplied by this after collision

	bool mustFollow = true;

    Sprite* editorSprite = nullptr;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

private:
	EditorObject* _target = nullptr;

	vec2 velocity = vec2(0);
};

