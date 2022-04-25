#pragma once
#include "EditorObject.h"

/// <summary>
/// Props of diferent types of laser
/// </summary>
struct LaserSharedProps
{
	vec4 centerColor;
	vec4 borderColor;
};

class Laser : public EditorObject
{
public:
	enum class LaserType { noTeleprot, disableTeleport, lastValue = disableTeleport };

	const float width = 0.4f;
	const float editorSpriteAlpha = 0.5f;

	const LaserSharedProps props[2] = {
		LaserSharedProps {
			vec4(1, 0.8, 0.9, 1), // centerColor
			vec4(1, 0.2, 0.7, 0), // borderColor
		},
		LaserSharedProps {
			vec4(0.8, 0.9, 1, 1),// centerColor
			vec4(0.3, 0.6, 1, 0),// borderColor
		},
	};

	Laser();
	~Laser();

	LaserType laserType = LaserType::noTeleprot;

	Sprite* displaySprite = nullptr;
	Sprite* editorSprite = nullptr;

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

	virtual Laser* Copy() override;

	virtual vec2 DrawProperties(vec3 startPos) override;

	void SetType(LaserType newType);

private:
	LinkedListElement<std::function<void()>>* mainLoopFuncPos;

	void OnMainLoop();
	void SubscribeToFuncs();
	static void SetSpriteUniforms(Shader* shader, void* object);
};
