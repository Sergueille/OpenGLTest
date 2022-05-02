#pragma once

#include <list>

#include "Light.h"

class LightManager
{
public:
	static std::list<Light*> lights;

	static int pixelsPerUnit;

	static void BakeLight();
	static unsigned int GetLightData();

	static void ForceRefreshLightmaps();

	static vec2 lightmapMin;
	static vec2 lightmapMax;

private:
	static unsigned int texID;
	static std::string texLevelPath;

};
