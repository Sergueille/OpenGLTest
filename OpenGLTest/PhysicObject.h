#pragma once

#include "Collider.h"
#include "LinkedList.h"

using namespace glm;

class PhysicObject
{
public:
	PhysicObject(Collider* coll);
	virtual ~PhysicObject();

	/// <summary>
	/// Max angle allowed btw normal and up vector to consideer the surface as walkable
	/// </summary>
	const float maxAngleForWalkableSurface = 50;
	/// <summary>
	/// g, in units.s-2
	/// </summary>
	const float gravityAcceleration = 50;

	bool physicsEnabled = true;

	/// <summary>
	/// If true, pause all the physic simulation, used for pause menu
	/// </summary>
	static bool disableAllPhysics;

	vec2 velocity = vec2(0);
	bool isColliding = false;
	bool isOnWalkableSurface = false;

	Collider* collider = nullptr;

	vec2 GetPos();
	void SetPos(vec2 pos);

	void SubscribeToMainLoop();

protected:
	virtual void OnBeforeMove();
	virtual void OnAfterMove();

private:
	vec2 position = vec2(0);

	void OnMainLoop();
	void ReactToCollision(vec3 data);

	LinkedListElement<std::function<void()>>* funcPos = nullptr;
};
