#include "MenuManager.h"

#include "EventManager.h"
#include "Editor.h"
#include "TextManager.h"
#include "TweenManager.h"
#include "LightManager.h"
#include "PhysicObject.h"
#include "LogicRelay.h"
#include "LocalizationManager.h"
#include <soloud.h>

MenuManager::Menu MenuManager::currentMenu = Menu::none;
bool MenuManager::isSetup = false;

MenuManager::Menu MenuManager::previousMenu = Menu::none;

std::string MenuManager::focusedTextInputID = "";
std::string MenuManager::focusedTextInputValue = "";

const vec4 MenuManager::textColor = vec4(1);
const vec4 MenuManager::textDisabledColor = vec4(0.8, 0.8, 0.8, 1);
const vec4 MenuManager::textHoveredColor = vec4(1.15, 1.15, 1.15, 1);
const vec4 MenuManager::textSelectedColor = vec4(1.3, 1.3, 1.3, 1);

const vec4 MenuManager::textInputPlaceholderColor = vec4(0.8, 0.8, 0.8, 1);
const vec4 MenuManager::textInputBackgroundColor = vec4(0.1, 0.1, 0.1, 1);

bool MenuManager::escPressedLastFrame = false;
bool MenuManager::hoverLastFrame = false;
bool MenuManager::hoverThisFrame = false;
bool MenuManager::clickLastFrame = false;
float MenuManager::backspaceNextTime = 0;

std::string MenuManager::focusSound = "focus.wav";
std::string MenuManager::clickSound = "click.wav";
std::string MenuManager::blurSound = "blur.wav";
float MenuManager::uiSoundsVolume = 0.3f;

void MenuManager::Setup()
{
	if (isSetup) return;

	EventManager::OnMainLoop.push_end(OnMainLoop);
	EventManager::OnCharPressed.push_end(OnCaracterInput);

	isSetup = true;
}

void MenuManager::OnMainLoop()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
		escPressedLastFrame = false;

	HandleInputBackspace();

	hoverThisFrame = false;

	if (currentMenu == Menu::main)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		bool pressed;

		drawPos.y -= TextManager::RenderText("Title here", drawPos, titleScale, TextManager::right, textColor).y + margin;

		drawPos.y -= Button(drawPos, "new_game", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::newGame);
		}

		drawPos.y -= Button(drawPos, "load", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::load);
		}

		drawPos.y -= Button(drawPos, "level_editor", &pressed).y + margin;
		if (pressed)
		{
			EditorSaveManager::ClearGameLevel();
			OpenMenu(Menu::none);
			Editor::OpenEditor();
		}

		drawPos.y -= Button(drawPos, "quit", &pressed).y + margin;
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

		drawPos.y -= LocalText("paused_label", drawPos, titleScale).y + margin;

		drawPos.y -= Button(drawPos, "resume", &pressed).y + margin;
		if (pressed || (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressedLastFrame))
		{
			OpenMenu(Menu::ingame);
			PhysicObject::disableAllPhysics = false;
			escPressedLastFrame = true;
		}

		Button(drawPos, "back_to_main", &pressed);
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

		drawPos.y -= LocalText("load", drawPos, titleScale).y + margin;

		for (auto fileName = EditorSaveManager::userSaves.begin(); fileName != EditorSaveManager::userSaves.end(); fileName++)
		{
			drawPos.y -= Button(drawPos, *fileName, &pressed, true, true).y;

			if (pressed)
			{
				EditorSaveManager::LoadUserSave(*fileName);
			}
		}

		drawPos.y -= margin;
		PreviousMenuButton(drawPos);
	}
	else if (currentMenu == Menu::newGame)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		bool pressed;

		drawPos.y -= LocalText("new_game", drawPos, titleScale).y + margin;
		LocalText("new_game_label", drawPos, textSize);
		drawPos.y -= smallMargin;

		std::string fileName = "";
		drawPos.y -= TextInput(drawPos, &fileName, "filename_placeholder", "loadPath").y;
		drawPos.y -= textSize + smallMargin;

		drawPos.y -= Button(drawPos, "play", &pressed, fileName != "").y + margin;
		if (pressed)
		{
			EditorSaveManager::currentUserSave = fileName + ".sav";
			EditorSaveManager::LoadLevelWithTransition("tuto.map", [] { OpenMenu(Menu::ingame); });
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
		TextManager::RenderText("Unknown menu type!", vec3(screenMargin, screenMargin, UIbaseZPos), 15);
	}

	// Handle focus sounds
	if (hoverThisFrame && !hoverLastFrame)
	{
		Utility::PlaySound(focusSound, uiSoundsVolume);
	}
	else if (!hoverThisFrame && hoverLastFrame)
	{
		Utility::PlaySound(blurSound, uiSoundsVolume);
	}

	hoverLastFrame = hoverThisFrame;
}

void MenuManager::OnCaracterInput(GLFWwindow* window, unsigned int codepoint)
{
	if (focusedTextInputID != "")
	{
		focusedTextInputValue += char(codepoint);
	}
}

vec2 MenuManager::TextInput(vec3 drawPos, std::string* value, std::string placeHolderKey, std::string ID)
{
	vec2 inputRect = vec2(textInputWidth, textInputHeight);
	Sprite(drawPos + vec3(0, -inputRect.y, -1), drawPos + vec3(inputRect.x, 0, -1), textInputBackgroundColor).Draw();

	float margin = (inputRect.y - textSize) / 2;
	vec3 textPos = drawPos + vec3(margin, -textSize-margin, 0);

	if (focusedTextInputID == ID)
	{
		*value = focusedTextInputValue;

		vec2 textRect = TextManager::RenderText(*value, textPos, textSize, TextManager::right, textSelectedColor);

		bool showCursor = float(static_cast<int>(Utility::time * 1000) % textInputCursorBlink) > (textInputCursorBlink / 2.f);
		if (showCursor)
		{
			// Draw cursor
			vec2 startPos = vec2(textPos) + textRect;
			vec2 endPos = startPos + vec2(2, -textSize);
			Sprite(vec3(startPos.x, startPos.y, drawPos.z + 1), vec3(endPos.x, endPos.y, drawPos.z + 1), textColor).Draw();
		}
		
		return inputRect;
	}
	else
	{
		std::u8string placeHolder = LocalizationManager::GetLocale(placeHolderKey);
		std::u8string displayString = value->length() == 0 ? placeHolder : std::u8string(value->begin(), value->end());

		vec2 mousePos = Utility::GetMousePos();
		mousePos.y *= -1;
		mousePos.y += Utility::screenY;
		

		bool hoverX = mousePos.x > drawPos.x && mousePos.x < drawPos.x + inputRect.x;
		bool hoverY = mousePos.y > drawPos.y - inputRect.y && mousePos.y < drawPos.y;

		vec4 color;
		if (hoverX && hoverY)
		{
			if (glfwGetMouseButton(Utility::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				focusedTextInputID = ID;
				focusedTextInputValue = "";
			}

			color = textHoveredColor;
		}
		else
		{
			color = value->length() == 0 ? textInputPlaceholderColor : textColor;
		}

		if (glfwGetMouseButton(Utility::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			if (hoverX && hoverY)
			{
				if (focusedTextInputID != ID)
				{
					focusedTextInputID = ID;
					focusedTextInputValue = "";
				}
			}
			else
			{
				focusedTextInputID = "";
				focusedTextInputValue = "";
			}

			hoverThisFrame = true;
		}

		TextManager::RenderText(displayString, textPos, textSize, TextManager::right, color);
		return inputRect;
	}
}

vec2 MenuManager::Button(vec3 drawPos, std::string key, bool* out, bool enabled, bool noLocale)
{
	std::u8string text = noLocale ? std::u8string(key.begin(), key.end()) : LocalizationManager::GetLocale(key);

	vec4 color;
	vec2 textRect = TextManager::GetRect(text, textSize);

	if (!enabled)
	{
		color = textDisabledColor;
		*out = false;
	}
	else
	{
		vec2 mousePos = Utility::GetMousePos();
		mousePos.y *= -1;
		mousePos.y += Utility::screenY;

		bool hoverX = mousePos.x > drawPos.x && mousePos.x < drawPos.x + textRect.x;
		bool hoverY = mousePos.y > drawPos.y && mousePos.y < drawPos.y + textRect.y;

		*out = false;

		if (hoverX && hoverY)
		{
			if (glfwGetMouseButton(Utility::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				color = textSelectedColor;
				clickLastFrame = true;
			}
			else
			{
				color = textHoveredColor;

				if (clickLastFrame)
				{
					*out = true;
					Utility::PlaySound(clickSound, uiSoundsVolume);
				}

				clickLastFrame = false;
			}

			hoverThisFrame = true;
		}
		else
		{
			color = textColor;
		}
	}

	TextManager::RenderText(text, drawPos, textSize, TextManager::right, color);
	return textRect;
}

vec2 MenuManager::LocalText(std::string key, glm::vec3 pos, float scale)
{
	return TextManager::RenderText(
		LocalizationManager::GetLocale(key),
		pos, scale, TextManager::right, textColor, false
	);
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
		Camera::getTarget = nullptr;

		if (EditorSaveManager::filePath != "menu_bg.map")
		{
			EditorSaveManager::LoadLevel("menu_bg.map", false);
		}
	}
}

vec2 MenuManager::PreviousMenuButton(vec3 drawPos)
{
	bool pressed;
	vec2 size = Button(drawPos, "back", &pressed);

	if (pressed || (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressedLastFrame))
	{
		OpenMenu(previousMenu);
		escPressedLastFrame = true;
		Utility::PlaySound("back.wav", uiSoundsVolume);
	}

	return size;
}

void MenuManager::HandleInputBackspace()
{
	if (focusedTextInputID != "")
	{
		if (glfwGetKey(Utility::window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		{
			// Control: remove word
			if (glfwGetKey(Utility::window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(Utility::window, GLFW_KEY_RIGHT_CONTROL))
			{
				if (backspaceNextTime < Utility::time)
				{
					if (focusedTextInputValue.length() > 0)
					{
						focusedTextInputValue.pop_back();
					}

					while (focusedTextInputValue.length() > 0 && isalpha(focusedTextInputValue[focusedTextInputValue.length() - 1]))
					{
						focusedTextInputValue.pop_back();
					}

					backspaceNextTime = Utility::time + (backspaceFirstLatency / 1000.f);
				}
			}
			else
			{
				// remove last letter
				if (focusedTextInputValue.length() > 0 && backspaceNextTime < Utility::time)
				{
					focusedTextInputValue.pop_back();
					if (backspaceNextTime == 0)
						backspaceNextTime = Utility::time + (backspaceFirstLatency / 1000.f);
					else
						backspaceNextTime = Utility::time + (backspaceLatency / 1000.f);
				}
			}
		}
		else
		{
			backspaceNextTime = 0;
		}
	}
}
