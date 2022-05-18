#include "MenuManager.h"

#include "EventManager.h"
#include "Editor.h"
#include "TextManager.h"

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
