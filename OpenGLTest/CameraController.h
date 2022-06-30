#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "TweenManager.h"

using namespace glm;

constexpr int CAM_CONTROL_EVENT_COUNT = 2;

class CameraController : public EditorObject
{
public:
	CameraController();
	~CameraController();

    Sprite* editorSprite = nullptr;

	/// <summary>
	/// Proportion of the default zoom
	/// </summary>
	float zoom = 1.f;
	/// <summary>
	/// Duration of the zoom transition
	/// </summary>
	float transTime = 2.f;

	float shakeIntensity = 1;
	float shakeDuration = 2;

	static ObjectEvent events[CAM_CONTROL_EVENT_COUNT];

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	/// <summary>
	/// Set camera zoom
	/// </summary>
	/// <param name="zoom">Proportion of the default zoom</param>
	void SetZoom(float zoom, float transTime);

	/// <summary>
	/// Set camera zoom with objects props
	/// </summary>
	void SetZoomWithProps();

	void Shake();

private:
	TweenAction<float>* zoomTweenAction = nullptr;
};

