#pragma once
#include "EditorObject.h"
#include "RectCollider.h"

constexpr int LASER_EVENT_COUNT = 3;

class Laser : public EditorObject
{
public:
	enum class LaserType { noTeleport, deadlyLaser, lastValue = deadlyLaser };

	const float width = 0.5f;
	const float editorSpriteAlpha = 0.5f;

	/// <summary>
	/// The laser will not update if the player is further than this distance
	/// </summary>
	const float minPlayerDist = 30;

	Laser();
	~Laser();

	static std::list<Laser*> lasers;

	bool laserOn = true;
	bool startOn = true;

	LaserType laserType = LaserType::noTeleport;

	Sprite* displaySprite = nullptr;
	Sprite* editorSprite = nullptr;
	Sprite* emmiterSprite = nullptr;
	Sprite* glowSprite = nullptr;

	RectCollider* laserCollider;

	/// <summary>
	/// The offset of the display sprite at the start of the laser
	/// </summary>
	float startOffset = -0.1f;
	/// <summary>
	/// The offset of the display sprite at the end of the laser
	/// </summary>
	float endOffset = 0.05f;

	virtual void UpdateTransform() override;

	virtual void Save() override;
	virtual void Load(std::map<std::string, std::string>* props) override;

	virtual void Enable() override;
	virtual void Disable() override;

	void TurnOn();
	void TurnOff();
	void ToggleOnOff();

	virtual Laser* Copy() override;

	virtual vec2 DrawProperties(vec3 startPos) override;

	void SetType(LaserType newType);

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

private:
	LinkedListElement<std::function<void()>>* laserMainLoopFuncPos = nullptr;

	vec2 intersectionPos = vec2(0);

	bool hasRefreshedOnce = false;

	void OnLaserMainLoop();
	static void SetSpriteUniforms(Shader* shader, void* object);

	static const ObjectEvent events[LASER_EVENT_COUNT];
};
