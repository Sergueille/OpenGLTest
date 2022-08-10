#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

constexpr int TEMP_MANAGER_EVENT_COUNT = 3;

class TemperatureManager : public EditorObject
{
public:
	TemperatureManager();
	~TemperatureManager();

	int startTemperature;
	int maxTemperature;
	int duration;
	int subAmount;
	int subDuration;
	std::string localKey;
	vec3 textColor;
	EventList onReachMax;

	static ObjectEvent events[TEMP_MANAGER_EVENT_COUNT];
    Sprite* editorSprite = nullptr;

	int currentTemp;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	void Start();
	void Sub();
	void Stop();

private:
	bool isActive = false;
	float startTime;
	bool eventAlreadySent = false;
};
