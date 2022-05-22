#include "MenuManager.h"

#include "EventManager.h"
#include "Editor.h"
#include "TextManager.h"
#include "TweenManager.h"
#include "LightManager.h"
#include "PhysicObject.h"

MenuManager::Menu MenuManager::currentMenu = Menu::none;
bool MenuManager::isSetup = false;
bool MenuManager::escPressedLastFrame = false;

MenuManager::Menu MenuManager::previousMenu = Menu::none;

std::string MenuManager::focusedTextInputID = "";
std::string MenuManager::focusedTextInputValue = "";

const vec4 MenuManager::textInputPlaceholderColor = vec4(0.8, 0.8, 0.8, 1);
const vec4 MenuManager::textInputTextColor = vec4(1);
const vec4 MenuManager::textInputHoverColor = vec4(1.3, 1.3, 1.3, 1);
const vec4 MenuManager::textInputEditColor = vec4(1.15, 1.15, 1.15, 1);
const vec4 MenuManager::textInputBackgroundColor = vec4(0.2, 0.2, 0.2, 0.2);

void MenuManager::Setup()
{
	if (isSetup) return;

	EventManager::OnMainLoop.push_end(OnMainLoop);
	isSetup = true;
}

void MenuManager::OnMainLoop()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
		escPressedLastFrame = false;

	if (currentMenu == Menu::main)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		bool pressed;

		drawPos.y -= TextManager::RenderText("Title here", drawPos, titleScale).y + margin;

		drawPos.y -= Editor::UIButton(drawPos, "New game", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::newGame);
		}

		drawPos.y -= Editor::UIButton(drawPos, "Load", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::load);
		}

		drawPos.y -= Editor::UIButton(drawPos, "Open level editor", &pressed).y + margin;
		if (pressed)
		{
			EditorSaveManager::ClearGameLevel();
			OpenMenu(Menu::none);
			Editor::OpenEditor();
		}

		drawPos.y -= Editor::UIButton(drawPos, "Quit", &pressed).y + margin;
		if (pressed)
		{
			glfwSetWindowShouldClose(window, 1);
		}
	}
	else if (currentMenu == Menu::paused)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		bool pressed;

		Sprite(vec3(0, 0, Editor::UIBaseZPos - 5), vec3(screenX, screenY, Editor::UIBaseZPos - 5), vec4(0, 0, 0, 0.3)).Draw();

		drawPos.y -= TextManager::RenderText("The game is paused", drawPos, titleScale).y + margin;

		drawPos.y -= Editor::UIButton(drawPos, "Resume", &pressed).y + margin;
		if (pressed || (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressedLastFrame))
		{
			OpenMenu(Menu::ingame);
			PhysicObject::disableAllPhysics = false;
			escPressedLastFrame = true;
		}

		Editor::UIButton(drawPos, "Back to main menu", &pressed);
		if (pressed)
		{
			EditorSaveManager::ClearGameLevel();
			OpenMenu(Menu::main);
			PhysicObject::disableAllPhysics = false;
		}
	}
	else if (currentMenu == Menu::ingame)
	{
		// Escape: open pause menu
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressedLastFrame)
		{
			OpenMenu(Menu::paused);
			PhysicObject::disableAllPhysics = true;
			escPressedLastFrame = true;
		}
	}
	else if (currentMenu == Menu::load)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		bool pressed;

		drawPos.y -= TextManager::RenderText("Load a save file:", drawPos, titleScale).y + margin;

		for (auto fileName = EditorSaveManager::userSaves.begin(); fileName != EditorSaveManager::userSaves.end(); fileName++)
		{
			drawPos.y -= Editor::UIButton(drawPos, *fileName, &pressed).y;

			if (pressed)
			{
				EditorSaveManager::LoadUserSave("Saves\\" + *fileName);
			}
		}

		drawPos.y -= margin;
		PreviousMenuButton(drawPos);
	}
	else if (currentMenu == Menu::newGame)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		bool pressed;

		drawPos.y -= TextManager::RenderText("Start a new game:", drawPos, titleScale).y + margin;

		drawPos.y -= TextManager::RenderText("Please choose a name for your save file:", drawPos, textSize).y;

		std::string fileName = "";
		drawPos.y -= TextInput(drawPos, &fileName, "File name...", "loadPath").y;

		if (Editor::focusedTextInputID == "loadPath")
		{
			drawPos.y -= TextManager::RenderText("Press enter to start playing", drawPos, textSize).y;
		}

		if (fileName != "")
		{
			EditorSaveManager::LoadLevelWithTransition("intro_walls.map", [] { OpenMenu(Menu::ingame); });
		}

		drawPos.y -= margin;
		PreviousMenuButton(drawPos).y;
	}
	else if (currentMenu == Menu::none)
	{
		// Nothing!
	}
	else
	{
		TextManager::RenderText("Unknown menu type!", vec3(screenMargin, screenMargin, UIbaseZPos), 10);
	}
}

vec2 MenuManager::TextInput(vec3 drawPos, std::string* value, std::string placeHolder, std::string ID)
{
	vec2 inputRect = vec2(textInputWidth, textInputHeight);
	Sprite(drawPos, drawPos + vec3(inputRect.x, inputRect.y, -1), textInputBackgroundColor).Draw();

	if (focusedTextInputID == ID)
	{
		if (glfwGetKey(Utility::window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			*value = focusedTextInputValue;
			focusedTextInputID = "";
			focusedTextInputValue = "";
		}
		if (glfwGetKey(Utility::window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			focusedTextInputID = "";
		}

		bool showCursor = float(static_cast<int>(Utility::time * 1000) % textInputCursorBlink) > (textInputCursorBlink / 2.f);
		// TODO: draw cursor
		vec2 textRect = TextManager::RenderText(*value, drawPos, textSize, TextManager::right, textInputEditColor);
		return textRect;
	}
	else
	{
		std::string displayString = value->length() == 0 ? placeHolder : *value;

		vec2 mousePos = Utility::GetMousePos();
		mousePos.y *= -1;
		mousePos.y += Utility::screenY;
		

		bool hoverX = mousePos.x > drawPos.x && mousePos.x < drawPos.x + inputRect.x;
		bool hoverY = mousePos.y > drawPos.y && mousePos.y < drawPos.y + inputRect.y;

		vec4 color;
		if (hoverX && hoverY)
		{
			if (glfwGetMouseButton(Utility::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				focusedTextInputID = ID;
				focusedTextInputValue = "";
			}

			color = textInputHoverColor;
		}
		else
		{
			color = value->length() == 0 ? textInputPlaceholderColor : textInputTextColor;
		}

		TextManager::RenderText(displayString, drawPos, textSize, TextManager::right, color);
		return inputRect;
	}
}

MenuManager::Menu MenuManager::GetCurrentMenu()
{
	return currentMenu;
}

void MenuManager::OpenMenu(Menu menu)
{
	if (!isSetup) Setup();

	previousMenu = currentMenu;
	currentMenu = menu;

	if (currentMenu == Menu::load)
	{
		EditorSaveManager::IndexUserSaves();
	}
	if (currentMenu == Menu::main)
	{
		if (EditorSaveManager::filePath != "menu_bg.map")
		{
			EditorSaveManager::LoadLevel("menu_bg.map", false);
			Camera::position = vec2(0, 0);
		}
	}
}

vec2 MenuManager::PreviousMenuButton(vec3 drawPos)
{
	bool pressed;
	vec2 size = Editor::UIButton(drawPos, "Back", &pressed);

	if (pressed || (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressedLastFrame))
	{
		OpenMenu(previousMenu);
		escPressedLastFrame = true;
	}

	return size;
}
