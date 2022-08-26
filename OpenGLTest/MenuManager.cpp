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
#include "SettingsManager.h"
#include "InputManager.h"

MenuManager::Menu MenuManager::currentMenu = Menu::none;
bool MenuManager::isSetup = false;

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

int MenuManager::selectedChapter = 0;

std::list<MenuManager::Menu> MenuManager::menuPath = std::list<Menu>();

float MenuManager::buttonTransitionValue = 0;

int MenuManager::setKeyID = -1;
bool MenuManager::setKeyPrim = false;

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

	bool pressed;
	hoverThisFrame = false;

	if (currentMenu == Menu::main)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		drawPos.y -= TextManager::RenderText(SettingsManager::gameConfig["gameName"], drawPos, titleScale, TextManager::right, textColor).y + margin;

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
		
		drawPos.y -= Button(drawPos, "options", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::options);
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
		drawPos.y -= LocalText("load", drawPos, titleScale).y + margin;

		for (auto fileName = EditorSaveManager::userSaves.begin(); fileName != EditorSaveManager::userSaves.end(); fileName++)
		{
			drawPos.y -= Button(drawPos, *fileName, &pressed, true, true).y + margin;

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
		drawPos.y -= LocalText("new_game", drawPos, titleScale).y + margin;
		LocalText("new_game_label", drawPos, textSize);
		drawPos.y -= smallMargin;

		// Draw input field
		std::string fileName = "";
		drawPos.y -= TextInput(drawPos, &fileName, "filename_placeholder", "loadPath").y;
		drawPos.y -= textSize + margin;

		// Draw chapters
		int unlockedChapters = std::stoi(SettingsManager::progress["unlockedChapters"]);
		int drawX = (int)drawPos.x;
		int chapterIconWidth = chapterInconHeight * 16 / 9;
		int chapterCount = std::stoi(SettingsManager::gameConfig["chapterCount"]);
		int maxChaptersInLine = 3;
		int lineHeight = chapterInconHeight + textSize + margin;
		for (int i = 0; i < chapterCount; i++)
		{
			// Line break
			if (i > 0 && i % maxChaptersInLine == 0)
			{
				drawPos.x = (float)drawX;
				drawPos.y -= lineHeight;
			}

			bool hover = IsMouseInBox(
				drawPos + vec3(0, textSize, 0),
				drawPos + vec3(chapterIconWidth, -chapterInconHeight - smallMargin, 0));

			if (i < unlockedChapters && hover && glfwGetMouseButton(window, 0) == GLFW_PRESS)
				selectedChapter = i;

			// Choose color
			vec4 color;
			if (i == selectedChapter)
				color = vec4(1.2, 1.2, 1.2, 1);
			else if (i < unlockedChapters && hover)
				color = vec4(1);
			else if (i < unlockedChapters)
				color = vec4(0.7, 0.7, 0.7, 1);
			else
				color = vec4(0.3, 0.3, 0.3, 0.5);

			// Chapter name
			std::string locale = LocalizationManager::GetLocale("chapter") + " " + std::to_string(i + 1);
			TextManager::RenderText(locale, drawPos, textSize, TextManager::right, color, false).y;

			// Chapter image
			int shiftX = (chapterIconWidth - chapterInconHeight) / 2;
			Sprite icon = Sprite(
				vec3(drawPos.x + shiftX, drawPos.y - chapterInconHeight - smallMargin, UIbaseZPos),
				vec3(drawPos.x + shiftX + chapterInconHeight, drawPos.y - smallMargin, UIbaseZPos),
				color
			);
			icon.texture = RessourceManager::GetTexture("Chapters\\" + std::to_string(i) + ".png");
			icon.Draw();

			drawPos.x += chapterIconWidth + smallMargin;
		}
		drawPos.x = (float)drawX;
		drawPos.y -= lineHeight;

		// Draw buttons
		drawPos.y -= margin;
		drawPos.y -= Button(drawPos, "play", &pressed, fileName != "").y + margin;
		if (pressed)
		{
			EditorSaveManager::currentUserSave = fileName + ".sav";

			std::string mapKey = "chapterMap" + std::to_string(selectedChapter);
			std::string map = SettingsManager::gameConfig[mapKey];
			EditorSaveManager::LoadLevelWithTransition(map, [] { OpenMenu(Menu::ingame); });
		}
		PreviousMenuButton(drawPos).y;
	}
	else if (currentMenu == Menu::options)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		drawPos.y -= LocalText("options", drawPos, titleScale).y + margin;

		drawPos.y -= Button(drawPos, "graphics", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::graphics);
		}

		drawPos.y -= Button(drawPos, "controls", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::controls);
		}

		drawPos.y -= Button(drawPos, "audio", &pressed).y + margin;
		if (pressed)
		{
			OpenMenu(Menu::audio);
		}

		drawPos.y -= SettingToggle(drawPos, "display_fps", "displayFPS").y + margin;

		LocalText("language", drawPos, textSize);
		drawPos.y -= Button(drawPos + vec3(propsScale, 0, 0), "language_name", &pressed).y;

		if (pressed)
		{
			int langID = (int)LocalizationManager::currentLanguage;
			langID++;
			langID %= (int)LocalizationManager::Language::langCount;
			SettingsManager::SetIntSetting("language", langID);
			LocalizationManager::LoadLanguage((LocalizationManager::Language)langID);
		}

		drawPos.y -= margin; 
		Button(drawPos, "back", &pressed).x;
		if (pressed)
		{
			SettingsManager::SaveSettings();
			PreviousMenu();
		}
	}
	else if (currentMenu == Menu::graphics)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		drawPos.y -= LocalText("graphics", drawPos, titleScale).y + margin;

		drawPos.y -= SettingToggle(drawPos, "fullscreen", "fullscreen").y + margin;

		if (SettingsManager::settings["fullscreen"] == "1")
		{
			int count;
			glfwGetMonitors(&count);

			if (count > 1)
			{
				bool useMain = SettingsManager::settings["monitor"] == "main";
				drawPos.y -= Toggle(drawPos, "use_main", &useMain).y + margin;

				if (useMain)
				{
					SettingsManager::settings["monitor"] = "main";
				}
				else
				{
					if (SettingsManager::settings["monitor"] == "main") SettingsManager::settings["monitor"] = "0";
					drawPos.y -= SettingIntSlider(drawPos, "monitor_id", "monitor", 0, count - 1).y + margin;
				}
			}
			else
			{
				SettingsManager::settings["monitor"] = "main";
			}
		}
		else
		{
			float res = SettingsManager::GetFloatSetting("screenY");
			drawPos.y -= Slider(drawPos, "win_size", &res, 540, 1080, false, true).y + margin;
			SettingsManager::SetIntSetting("screenY", (int)res);
			SettingsManager::SetIntSetting("screenX", (int)(res * 16.f / 9.f));
		}

		drawPos.y -= margin;

		drawPos.x += Button(drawPos, "apply", &pressed).x + margin;
		if (pressed)
		{
			SettingsManager::SaveSettings();
			SettingsManager::CreateGLFWWindow();
		}

		PreviousMenuButton(drawPos);
	}
	else if (currentMenu == Menu::audio)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		drawPos.y -= LocalText("audio", drawPos, titleScale).y + margin;

		drawPos.y -= SettingSlider(drawPos, "global_volume", "globalVolume", 0, 2, true).y + margin;
		drawPos.y -= SettingSlider(drawPos, "sounds_volume", "gameSoundsVolume", 0, 2, true).y + margin;
		drawPos.y -= SettingSlider(drawPos, "music_volume", "musicVolume", 0, 2, true).y + margin;
		drawPos.y -= SettingSlider(drawPos, "ui_volume", "uiVolume", 0, 2, true).y + margin;

		SettingsManager::ApplySettings();

		drawPos.y -= margin;

		Button(drawPos, "back", &pressed);
		if (pressed)
		{
			SettingsManager::SaveSettings();
			PreviousMenu();
		}
	}
	else if (currentMenu == Menu::controls)
	{
		vec3 drawPos = vec3(screenMargin, screenY - screenMargin, UIbaseZPos);
		drawPos.y -= LocalText("controls", drawPos, titleScale).y + margin;

		if (setKeyID == -1)
		{
			for (int i = 0; i < (int)InputManager::KeyBinding::keyCount; i++)
			{
				std::string str_i = std::to_string(i);
				InputManager::KeyBinding keybind = (InputManager::KeyBinding)i;

				LocalText("action" + str_i, drawPos, textSize);

				Button(
					drawPos + vec3(propsScale, 0, 0),
					InputManager::GetKeyName(keybind, true),
					&pressed, true, true
				);

				if (pressed)
				{
					setKeyID = i;
					setKeyPrim = true;
				}

				Button(
					drawPos + vec3(2 * propsScale, 0, 0),
					InputManager::GetKeyName(keybind, false),
					&pressed, true, true
				);

				if (pressed)
				{
					setKeyID = i;
					setKeyPrim = false;
				}

				drawPos.y -= textSize + smallMargin;
			}

			drawPos.y -= margin;

			PreviousMenuButton(drawPos);
		}
		else
		{
			drawPos.x += LocalText("press_key", drawPos, textSize).x + smallMargin;
			LocalText("action" + std::to_string(setKeyID), drawPos, textSize);

			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				setKeyID = -1;
			else
			{
				for (int i = 0; i < 400; i++)
				{
					if (glfwGetKey(window, i) == GLFW_PRESS)
					{
						InputManager::SetKey((InputManager::KeyBinding)setKeyID, i, setKeyPrim);
						setKeyID = -1;
					}
				}
			}
		}
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

		TweenManager<float>::Tween(0, 1, (float)buttonBarTransition / 1000.f, [](float value) {
			buttonTransitionValue = value;
		}, cubicInOut);
	}
	else if (!hoverThisFrame && hoverLastFrame)
	{
		Utility::PlaySound(blurSound, uiSoundsVolume);
	}

	if (!hoverThisFrame)
	{
		buttonTransitionValue = 0;
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
		std::string placeHolder = LocalizationManager::GetLocale(placeHolderKey);
		std::string displayString = value->length() == 0 ? placeHolder : *value;

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
	std::string text = noLocale ? key : LocalizationManager::GetLocale(key);

	vec4 color;
	vec2 textRect = TextManager::GetRect(text, textSize);

	float shift = 0;

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

		if (IsMouseInBox(drawPos, drawPos + vec3(textRect.x, textRect.y, 0)))
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

			int barOverflow = (buttonBarHeight - textSize) / 2;

			Sprite(
				drawPos + vec3(0, textSize + barOverflow + buttonBarYShift, 0),
				drawPos + vec3(buttonBarWidth * buttonTransitionValue, -barOverflow + buttonBarYShift, 0),
				color
			).Draw();

			shift = (buttonBarWidth + buttonBarMargin) * buttonTransitionValue;

			hoverThisFrame = true;
		}
		else
		{
			color = textColor;
		}
	}

	TextManager::RenderText(text, drawPos + vec3(shift, 0, 0), textSize, TextManager::right, color);
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

void MenuManager::PreviousMenu()
{
	if (menuPath.size() == 0) return;

	OpenMenu(menuPath.back());
	menuPath.pop_back();
	menuPath.pop_back();
	Utility::PlaySound("back.wav", uiSoundsVolume);
}

void MenuManager::OpenMenu(Menu menu)
{
	if (!isSetup) Setup();

	menuPath.push_back(currentMenu);
	focusedTextInputID = "";

	currentMenu = menu;

	if (currentMenu == Menu::load)
	{
		EditorSaveManager::IndexUserSaves();
	}
	if (currentMenu == Menu::main)
	{
		Camera::getTarget = nullptr;

		if (EditorSaveManager::filePath != SettingsManager::gameConfig["menuMap"])
		{
			EditorSaveManager::LoadLevel(SettingsManager::gameConfig["menuMap"], false);
		}
	}
}

vec2 MenuManager::PreviousMenuButton(vec3 drawPos)
{
	bool pressed;
	vec2 size = Button(drawPos, "back", &pressed);

	if (pressed || (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressedLastFrame))
	{
		PreviousMenu();
		escPressedLastFrame = true;
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

vec2 MenuManager::Slider(vec3 drawPos, std::string label, float* value, float min, float max, bool percentage, bool useInt)
{
	int deltaStartX = propsScale + margin;
	int deltaEndX = propsScale + margin + sliderWidth;
	int middleY = ((int)drawPos.y + (int)drawPos.y + textSize) / 2;

	vec2 mousePos = Utility::GetMousePos();
	mousePos.y *= -1;
	mousePos.y += Utility::screenY;

	vec4 color;
	if (IsMouseInBox(drawPos + vec3(deltaStartX, 0, 0), drawPos + vec3(deltaEndX, textSize, 0)))
	{
		color = textHoveredColor;

		if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
		{
			float relativePos = (mousePos.x - drawPos.x - deltaStartX) / (deltaEndX - deltaStartX);
			*value = min + relativePos * (max - min);
			if (*value < min) *value = min;
			if (*value > max) *value = max;

			if (useInt) *value = std::round(*value);
		}

		hoverThisFrame = true;
	}
	else
	{
		color = textColor;
	}

	float normVal = (*value - min) / (max - min);
	float handleX = Lerp(drawPos.x + propsScale + margin, drawPos.x + deltaEndX, normVal);

	Sprite(
		vec3(drawPos.x + propsScale + margin, middleY - sliderHeight / 2, drawPos.y),
		vec3(drawPos.x + propsScale + margin + sliderWidth, middleY + sliderHeight / 2, drawPos.y),
		vec4(0.2, 0.2, 0.2, 0.5)
	).Draw();

	Sprite handle = Sprite(
		vec3(handleX - sliderHandleSize / 2, middleY - sliderHandleSize / 2, drawPos.z + 1),
		vec3(handleX + sliderHandleSize / 2, middleY + sliderHandleSize / 2, drawPos.z + 1),
		color
	);

	handle.texture = RessourceManager::GetTexture("Engine\\circle.png");
	handle.Draw();

	std::string local = LocalizationManager::GetLocale(label);
	TextManager::RenderText(local, drawPos, textSize, TextManager::right, color);

	char buffer[20];

	if (percentage)
		sprintf(buffer, "%d %%", static_cast<int>(*value * 100));
	else
	{
		if (useInt)
			sprintf(buffer, "%d", static_cast<int>(*value));
		else
			sprintf(buffer, "%.2f", *value);
	}


	std::string valueText(buffer);
	vec2 valueSize = TextManager::RenderText(valueText, drawPos + vec3(deltaEndX + margin, 0, 0), textSize, TextManager::right, color);

	return vec2(deltaEndX + margin + valueSize.x, textSize);
}

vec2 MenuManager::SettingSlider(vec3 drawPos, std::string label, std::string key, float min, float max, bool percentage)
{
	float val = SettingsManager::GetFloatSetting(key);
	vec2 size = Slider(drawPos, label, &val, min, max, percentage);
	SettingsManager::SetFloatSetting(key, val);
	return size;
}

vec2 MenuManager::SettingIntSlider(vec3 drawPos, std::string label, std::string key, int min, int max)
{
	int val = SettingsManager::GetIntSetting(key);
	float fval = static_cast<float>(val);
	vec2 size = Slider(drawPos, label, &fval, static_cast<float>(min), static_cast<float>(max), false, true);
	val = static_cast<int>(fval);
	SettingsManager::SetIntSetting(key, val);
	return size;
}

vec2 MenuManager::Toggle(vec3 drawPos, std::string label, bool* value)
{
	int middleY = ((int)drawPos.y + (int)drawPos.y + textSize) / 2;
	vec3 start = vec3(drawPos.x + propsScale, middleY - (toggleSize / 2), drawPos.z);
	vec3 end = vec3(drawPos.x + propsScale + toggleSize, middleY + (toggleSize / 2), drawPos.z);
	int middleX = ((int)drawPos.x + propsScale + (int)drawPos.x + propsScale + toggleSize) / 2;

	bool hover = IsMouseInBox(start, end);

	vec4 color;
	if (hover)
	{
		color = textHoveredColor;

		if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
		{
			clickLastFrame = true;
		}
		else
		{
			if (clickLastFrame)
			{
				*value = !(*value);
			}

			clickLastFrame = false;
		}

		hoverThisFrame = true;
	}
	else
	{
		color = textColor;
	}

	Sprite(
		start,
		end,
		vec4(0.2, 0.2, 0.2, 0.5)
	).Draw();

	if (*value || hover)
	{
		vec4 spriteColor = color;
		if (!(*value)) spriteColor = vec4(1, 1, 1, 0.2);

		Sprite(
			vec3(middleX - (toggleInnerSize / 2), middleY - (toggleInnerSize / 2), drawPos.z + 1),
			vec3(middleX + (toggleInnerSize / 2), middleY + (toggleInnerSize / 2), drawPos.z + 1),
			spriteColor
		).Draw();		
	}

	std::string local = LocalizationManager::GetLocale(label);
	TextManager::RenderText(local, drawPos, textSize, TextManager::right, color);

	return vec2(propsScale + toggleSize, textSize);
}

vec2 MenuManager::SettingToggle(vec3 drawPos, std::string label, std::string key)
{
	bool val = SettingsManager::settings[key] == "1";
	vec2 size = Toggle(drawPos, label, &val);
	SettingsManager::settings[key] = val ? "1" : "0";
	return size;
}

bool MenuManager::IsMouseInBox(vec2 start, vec2 end)
{
	vec2 mousePos = Utility::GetMousePos();
	mousePos.y *= -1;
	mousePos.y += Utility::screenY;

	if (start.x > end.x)
	{
		float tmp = end.x;
		end.x = start.x;
		start.x = tmp;
	}

	if (start.y > end.y)
	{
		float tmp = end.y;
		end.y = start.y;
		start.y = tmp;
	}

	return mousePos.x > start.x && mousePos.x < end.x&& mousePos.y > start.y && mousePos.y < end.y;
}
