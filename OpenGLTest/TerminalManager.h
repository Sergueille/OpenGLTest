#pragma once

#include <glm\common.hpp>
#include <vector>
#include <string>

using namespace glm;

constexpr float charPerSec = 50;

class TerminalManager
{
public:
	static const vec4 textColor; // color of the terminal text
	static const vec4 bgColor; // color of the terminal background
	static const int zPos = 90; // Position of the text on the z axis
	static const int textSize = 20; // Size of a line of text, in pixels
	static const int screenMargin = 10; // Plxels between the text and the border of the screen
	static const int nbLines = 6; // Nb of lines diplayed
	static const int maxCharInLine = 100; // Width of the terminal

	static float terminalWidth;

	static void Init();

	static std::vector<std::string> lines;

	/// <summary>
	/// Get all the text that must be displayed in the terminal
	/// </summary>
	static std::string GetAllText();
	/// <summary>
	/// Add text in temnial, should not contain new lines, they are added automatically
	/// </summary>
	static void Write(std::string text);
	/// <summary>
	/// Adds a new line!
	/// </summary>
	static void NewLine();

	static void ClearTerminal();

private:
	static void OnMainLoop();

	static std::string charsToWrite;// Characters waiting to be written
	static float writeStartTime; // Time when the last character was printed
	static int writtenChars;
};
