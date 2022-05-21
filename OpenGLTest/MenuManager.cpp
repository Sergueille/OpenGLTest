#include "MenuManager.h"

#include "EventManager.h"
#include "Editor.h"
#include "TextManager.h"
#include "TweenManager.h"
#include "LightManager.h"

MenuManager::Menu MenuManager::currentMenu = Menu::none;
bool MenuManager::isSetup = false;

void MenuManager::Setup()
{
	if (isSetup) return;

	EventManager::OnMainLoop.push_end(OnMainLoop);
	isSetup = true;
}

void MenuManager::OnMainLoop()
{
	if (currentMenu == Menu::main)
	{
		vec3 drawPos = vec3(screenMargin, screenMargin, UIbaseZPos);
		bool pressed;
		
		// Display button in reversed order
		drawPos.y += Editor::UIButton(drawPos, "Open level editor", &pressed).y + margin;

		if (pressed)
		{
			OpenMenu(Menu::none);
			Editor::OpenEditor();
		}

		drawPos.y += Editor::UIButton(drawPos, "Start game", &pressed).y + margin;

		if (pressed)
		{
			overlayZ = 2000;

			TweenManager<float>::Tween(0, 1, 2, [](float value) {
				overlayColor = vec4(0, 0, 0, value);
			}, linear)
			->SetOnFinished([] {
				// Load next level
				EditorSaveManager::LoadLevel("intro_walls.map", false);

				OpenMenu(Menu::none);

				// Fade out
				TweenManager<float>::Tween(1, 0, 2, [](float value) {
					overlayColor = vec4(0, 0, 0, value);
					}, linear)
					->SetCondition([] { return !LightManager::forceRefreshOnNextFrame; });

				// Set camera instantly
				if (Camera::getTarget != nullptr)
					Camera::position = Camera::getTarget();
			});
		}
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

MenuManager::Menu MenuManager::GetCurrentMenu()
{
	return currentMenu;
}

void MenuManager::OpenMenu(Menu menu)
{
	if (!isSetup) Setup();

	currentMenu = menu;
}
