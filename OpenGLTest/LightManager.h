#pragma once

#include <list>

#include "Light.h"
#include "ShadowCaster.h"

class LightManager
{
public:
	static std::list<Light*> lights;
	static std::list<ShadowCaster*> shadowCasters;

	static int pixelsPerUnit;

	static void BakeLight();
	static unsigned int GetLightData();

	static void ForceRefreshLightmaps();

	static vec2 lightmapMin;
	static vec2 lightmapMax;

	static bool forceRefreshOnNextFrame;
	static bool mustReadNewFile;
	static std::string texLevelPath;

private:
	static unsigned int texID;
};
