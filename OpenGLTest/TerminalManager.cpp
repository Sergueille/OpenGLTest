#include "TerminalManager.h"

#include "EventManager.h"
#include "TextManager.h"
#include "MenuManager.h"
#include "Editor.h"

const vec4 TerminalManager::textColor = vec4(0.1, 0.7, 0.1, 1);
const vec4 TerminalManager::bgColor = vec4(0.2f, 0.2f, 0.2f, 0.6f);
std::vector<std::string> TerminalManager::lines = std::vector<std::string>();

const std::string TerminalManager::botLogStart = "C:/process/log> ";
const std::string TerminalManager::communicationStart = "F:/message> ";

void TerminalManager::Init()
{
	EventManager::OnMainLoop.push_end(OnMainLoop);
	lines.push_back("");
}

std::string TerminalManager::GetAllText()
{
	std::string res = "";
	for (auto it = lines.begin(); it != lines.end(); it++)
	{
		res += *it + "\n";
	}

	return res;
}

void TerminalManager::Write(std::string text)
{
	int remaining = maxCharInLine - (int)lines[lines.size() - 1].size();
	if (text.size() > remaining) // Split line if it's too long
	{
		Write(text.substr(0, remaining));
		NewLine();
		Write(text.substr(remaining, text.size() - remaining));
	}
	else
	{
		lines[lines.size() - 1] += text;
	}
}

void TerminalManager::NewLine()
{
	lines.push_back("");

	if (lines.size() > nbLines) // If too many lines, remove first
		lines.erase(lines.begin());
}

void TerminalManager::OnMainLoop()
{
	if ((MenuManager::GetCurrentMenu() == MenuManager::Menu::ingame ||
		MenuManager::GetCurrentMenu() == MenuManager::Menu::none) && !Editor::enabled)
	{
		vec3 pos = vec3(screenMargin, Utility::screenY - screenMargin - textSize, zPos + 1); // Top-left corner pos
		vec2 size = TextManager::RenderText(GetAllText(), pos, textSize, TextManager::right, textColor); // Draw text

		// Draw background
		Sprite(
			vec3(0, Utility::screenY, zPos),
			vec3(maxCharInLine * textSize + 2 * screenMargin, Utility::screenY - 2 * screenMargin - size.y + textSize, zPos),
			bgColor
		).Draw();
	}
}
