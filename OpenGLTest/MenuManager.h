#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <string>

using namespace glm;

class MenuManager
{
	// TODO: currently using editor UI functions, but need to implement ingameUI specific ones after

public:
	enum class Menu { none, main, ingame, paused, load, newGame };

	const static int UIbaseZPos = 1000;
	const static int margin = 30;
	const static int smallMargin = 10;
	const static int screenMargin = 100;
	const static int titleScale = 40;

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

	static void OpenMenu(Menu menu);

	static Menu GetCurrentMenu();

private:
	static Menu currentMenu;
	static bool isSetup;

	static bool escPressedLastFrame;

	static Menu previousMenu;

	static std::string focusedTextInputID;
	static std::string focusedTextInputValue;

	static bool clickLastFrame;
	static float backspaceNextTime;

	static vec2 PreviousMenuButton(vec3 drawPos);

	static void Setup();
	static void OnMainLoop();
	static void OnCaracterInput(GLFWwindow* window, unsigned int codepoint);

	static void HandleInputBackspace();

	static vec2 TextInput(vec3 drawPos, std::string* value, std::string placeHolder, std::string ID);
	static vec2 Button(vec3 drawPos, std::string text, bool* out, bool enabled = true);
};

