#pragma once

#include <glm/glm.hpp>
#include <string>

using namespace glm;

class MenuManager
{
	// TODO: currently using editor UI functions, but need to implement ingameUI specific ones after

public:
	enum class Menu { none, main, ingame, paused, load, newGame };

	const static int UIbaseZPos = 1000;
	const static int margin = 15;
	const static int screenMargin = 50;
	const static int titleScale = 25;

	const static int textSize = 15;

	const static int textInputWidth = 300;
	const static int textInputHeight = 30;
	const static int textInputCursorBlink = 500; // Milliseconds
	const static vec4 textInputPlaceholderColor;
	const static vec4 textInputTextColor;
	const static vec4 textInputHoverColor;
	const static vec4 textInputEditColor;
	const static vec4 textInputBackgroundColor;

	static void OpenMenu(Menu menu);

	static Menu GetCurrentMenu();

private:
	static Menu currentMenu;
	static bool isSetup;

	static bool escPressedLastFrame;

	static Menu previousMenu;

	static std::string focusedTextInputID;
	static std::string focusedTextInputValue;

	static vec2 PreviousMenuButton(vec3 drawPos);

	static void Setup();
	static void OnMainLoop();

	static vec2 TextInput(vec3 drawPos, std::string* value, std::string placeHolder, std::string ID);
};

