#pragma once
#include "EditorObject.h"
#include "RectCollider.h"

constexpr int LASER_EVENT_COUNT = 3;

/// <summary>
/// Props of diferent types of laser
/// </summary>
struct LaserSharedProps
{
	vec4 centerColor;
	vec4 borderColor;
	vec2 noiseSpeed;
	float noiseSize;
	float distorsionAmount;
};

class Laser : public EditorObject
{
public:
	enum class LaserType { noTeleport, disableTeleport, lastValue = disableTeleport };

	const float width = 0.5f;
	const float editorSpriteAlpha = 0.5f;

	/// <summary>
	/// The laser will not update if the player is further than this distance
	/// </summary>
	const float minPlayerDist = 30;

	const LaserSharedProps props[2] = {
		LaserSharedProps {
			vec4(1, 0.8, 0.9, 1), // centerColor
			vec4(1, 0.3, 0.8, 0), // borderColor
			vec2(0.7f), // noiseSpeed
			0.3f, // noiseSize
			0.2f, // distorsionAmount
		},
		LaserSharedProps {
			vec4(0.8, 0.9, 1, 1),// centerColor
			vec4(0.3, 0.8, 1, 0),// borderColor
			vec2(0.7f), // noiseSpeed
			0.3f, // noiseSize
			0.2f, // distorsionAmount
		},
	};

	Laser();
	~Laser();

	static std::list<Laser*> lasers;

	bool laserOn = true;
	bool startOn = true;

	LaserType laserType = LaserType::noTeleport;

	Sprite* displaySprite = nullptr;
	Sprite* editorSprite = nullptr;

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

	bool hasRefreshedOnce = false;

	void OnLaserMainLoop();
	static void SetSpriteUniforms(Shader* shader, void* object);

	static const ObjectEvent events[LASER_EVENT_COUNT];
};
