#pragma once

#include <string>
#include <map>

constexpr int bloomResDivide = 2;

class SettingsManager
{
public:
	static std::map<std::string, std::string> settings;

	static unsigned int FBO;
	static unsigned int colorTex[3];
	static unsigned int pingpongFBO[2];
	static unsigned int pingpongBuffer[2];

	static bool windowCreated;

	static void ReadSettings();
	static void CreateGLFWWindow();
	static void SaveSettings();
	static void ApplySettings();

	static float GetFloatSetting(std::string key);
	static int GetIntSetting(std::string key);
	static void SetFloatSetting(std::string key, float val);
	static void SetIntSetting(std::string key, int val);
};

