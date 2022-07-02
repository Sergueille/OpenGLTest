#include "TerminalManager.h"

#include "EventManager.h"
#include "TextManager.h"
#include "MenuManager.h"
#include "Editor.h"
#include "LocalizationManager.h"

const vec4 TerminalManager::textColor = vec4(0.3, 0.8, 0.3, 1);
const vec4 TerminalManager::bgColor = vec4(0, 0, 0, 0.6f);
std::vector<std::string> TerminalManager::lines = std::vector<std::string>();
float TerminalManager::terminalWidth = 0;
std::string TerminalManager::charsToWrite = "";
float TerminalManager::writeStartTime = 0;
int TerminalManager::writtenChars = 0;

void TerminalManager::Init()
{
	EventManager::OnMainLoop.push_end(OnMainLoop);
	lines.push_back("");

	terminalWidth = maxCharInLine * TextManager::GetRect("a", textSize, true).x;
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

void TerminalManager::Write(std::string key)
{
	std::u8string textu8 = LocalizationManager::GetLocale("t_begin") + LocalizationManager::GetLocale(key);
	std::string text = std::string(textu8.begin(), textu8.end());

	if (lines[lines.size() - 1].length() > 0)
		NewLine();

	charsToWrite = text;
	writeStartTime = Utility::time;
	writtenChars = 0;
}

void TerminalManager::NewLine()
{
	lines.push_back("");

	if (lines.size() > nbLines) // If too many lines, remove first
		lines.erase(lines.begin());
}

void TerminalManager::ClearTerminal()
{
	lines.clear();
	lines.push_back("");
}

void TerminalManager::OnMainLoop()
{
	if ((MenuManager::GetCurrentMenu() == MenuManager::Menu::ingame ||
		MenuManager::GetCurrentMenu() == MenuManager::Menu::none) && !Editor::enabled)
	{
		std::string allText = GetAllText();

		if (allText.length() > 1)
		{
			vec3 pos = vec3(screenMargin, Utility::screenY - screenMargin - textSize, zPos + 1); // Top-left corner pos
			vec2 size = TextManager::RenderText(allText, pos, textSize, TextManager::right, textColor, true); // Draw text

			// Draw background
			Sprite(
				vec3(0, Utility::screenY, zPos),
				vec3(terminalWidth + 2 * screenMargin, Utility::screenY - 2 * screenMargin - size.y + textSize, zPos),
				bgColor
			).Draw();
		}

		if (charsToWrite.length() > 0)
		{
			int shouldWrite = static_cast<int>((Utility::time - writeStartTime) * (float)charPerSec);

			if (shouldWrite > charsToWrite.length()) 
				shouldWrite = (int)charsToWrite.length();

			for (int pos = writtenChars; pos < shouldWrite; pos++)
			{
				if (lines[lines.size() - 1].length() == maxCharInLine)
					NewLine();

				lines[lines.size() - 1] += charsToWrite[pos];
			}

			writtenChars = shouldWrite;

			if (writtenChars >= charsToWrite.length())
				charsToWrite = "";
		}
	}
}
