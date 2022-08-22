#include "TerminalManager.h"

#include "EventManager.h"
#include "TextManager.h"
#include "MenuManager.h"
#include "Editor.h"
#include "LocalizationManager.h"

const vec4 TerminalManager::textColor = vec4(1.f, 3.f, 1.f, 1.f); // Higher than real because the terminal background will be rendered on top of it
const vec4 TerminalManager::bgColor = vec4(0, 0, 0, 0.8f);
std::vector<std::string> TerminalManager::lines = std::vector<std::string>();
float TerminalManager::terminalWidth = 0;
std::string TerminalManager::charsToWrite = "";
float TerminalManager::writeStartTime = 0;
int TerminalManager::writtenChars = 0;

float TerminalManager::soundVolume = 0.5f;
float TerminalManager::maxPitch = 1.1f;
float TerminalManager::minPitch = 0.9f;
float TerminalManager::soundTime = 0.04f;
float TerminalManager::lastSoundTime = 0;

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
	std::string text = LocalizationManager::GetLocale("t_begin") + LocalizationManager::GetLocale(key);

	if (lines[lines.size() - 1].length() > 0)
		NewLine();

	// Add line breaks to prevent line overflow
	std::string newText = "";
	int lastSpacePos = 0;
	int lineLength = 0;
	for (int i = 0; i < text.length(); i++)
	{
		if (text[i] > 0) // Ignore UTF-8 semi-chars
			lineLength++;

		if (text[i] == ' ')
			lastSpacePos = i;

		if (lineLength >= maxCharInLine - 2)
		{
			lineLength = 0;

			if (lastSpacePos < i)
				newText = newText.substr(0, lastSpacePos) + "\n" + newText.substr(lastSpacePos + 1, newText.length() - lastSpacePos - 1);
			else
				newText = newText.substr(0, lastSpacePos) + "\n";
		}

		newText += text[i];
	}

	charsToWrite = newText;
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
			vec3 pos = vec3(screenMargin, Utility::screenY - screenMargin - textSize, zPos); // Top-left corner pos
			vec2 size = TextManager::RenderText(allText, pos, textSize, TextManager::right, textColor, true); // Draw text

			// Draw background
			//	 Drawn on top of text because, as a transparent sprite, it will be rendered after the text
			//	 If it's under the text, blending will be incorrect
			Sprite(
				vec3(0, Utility::screenY, zPos),
				vec3(terminalWidth + 2 * screenMargin, Utility::screenY - 2 * screenMargin - textSize * lines.size(), zPos + 1),
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
				if (charsToWrite[pos] == '\n')
					NewLine();
				else
					lines[lines.size() - 1] += charsToWrite[pos];
			}

			writtenChars = shouldWrite;

			if (writtenChars >= charsToWrite.length())
				charsToWrite = "";

			// Play sound
			if (lastSoundTime + soundTime < Utility::time)
			{
				lastSoundTime = Utility::time;

				float pitch = Lerp(minPitch, maxPitch, (float)rand() / (float)RAND_MAX);
				SoLoud::handle handle = Utility::PlaySound("keyboard1.wav", soundVolume * Utility::gameSoundsVolume);
				soloud->setRelativePlaySpeed(handle, pitch);
			}
		}
	}
}
