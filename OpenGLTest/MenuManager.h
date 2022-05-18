#pragma once

class MenuManager
{
	// TODO: currently using editor UI functions, but need to implement ingameUI specific ones after

public:
	enum class Menu { none, main };

	const static int UIbaseZPos = 1000;
	const static int margin = 15;
	const static int screenMargin = 50;

	static void OpenMenu(Menu menu);

	static Menu GetCurrentMenu();

private:
	static Menu currentMenu;
	static bool isSetup;

	static void Setup();
	static void OnMainLoop();
};

