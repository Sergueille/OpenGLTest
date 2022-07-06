#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <soloud.h>
#include <soloud_wav.h>
#include <list>

using namespace glm;

class MenuManager
{
public:
	enum class Menu { none, main, ingame, paused, load, newGame, options, graphics, audio };

	const static int UIbaseZPos = 1000;
	const static int margin = 30;
	const static int smallMargin = 10;
	const static int screenMargin = 100;
	const static int titleScale = 40;
	const static int propsScale = 200;

	const static int sliderWidth = 300;
	const static int sliderHeight = 15;
	const static int sliderHandleSize = 30;

	const static int textSize = 25;

	const static vec4 textColor;
	const static vec4 textDisabledColor;
	const static vec4 textHoveredColor;
	const static vec4 textSelectedColor;

	const static int textInputWidth = 300;
	const static int textInputHeight = 35;
	const static int textInputCursorBlink = 500; // Milliseconds
	const static vec4 textInputPlaceholderColor;
	const static vec4 textInputBackgroundColor;
	const static int backspaceFirstLatency = 300; // milliseconds
	const static int backspaceLatency = 30; // milliseconds

	static std::string focusSound;
	static std::string clickSound;
	static std::string blurSound;
	static float uiSoundsVolume;

	static std::list<Menu> menuPath;

	static void OpenMenu(Menu menu);
	static void PreviousMenu();

	static Menu GetCurrentMenu();

private:
	static Menu currentMenu;
	static bool isSetup;

	static std::string focusedTextInputID;
	static std::string focusedTextInputValue;

	static bool escPressedLastFrame;
	static bool hoverLastFrame;
	static bool hoverThisFrame;
	static bool clickLastFrame;
	static float backspaceNextTime;

	static vec2 PreviousMenuButton(vec3 drawPos);

	static void Setup();
	static void OnMainLoop();
	static void OnCaracterInput(GLFWwindow* window, unsigned int codepoint);

	static void HandleInputBackspace();

	static vec2 TextInput(vec3 drawPos, std::string* value, std::string placeHolderKey, std::string ID);
	static vec2 Button(vec3 drawPos, std::string key, bool* out, bool enabled = true, bool noLocale = false);
	static vec2 LocalText(std::string key, glm::vec3 pos, float scale);
	static vec2 Slider(vec3 drawPos, std::string label, float* value, float min = 0, float max = 1, bool percentage = false);
};

