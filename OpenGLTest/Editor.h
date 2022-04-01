#pragma once

#include "EventManager.h"
#include "EditorObject.h"
#include "TextManager.h"

#include <GLFW/glfw3.h>
#include <list>
#include <functional>

using namespace glm;

class EditorObject;
class Editor
{
public:
	static void SetupEditor();

	static std::list<EditorObject*> editorObjects;

	const static int panelSize = 300;
	const static int textSize = 25;
	const static int margin = 10;
	const static int indentation = 10;

	const static float selectClickMargin;

	const static int hoverHighlightDuration = 500; // milliseconds
	const static int textCursorBlink = 400; // milliseconds

	const static vec4 textColor;
	const static vec4 highlightColor;
	const static vec4 editColor;

	static std::string focusedTextInputID;
	static std::string focusedTextInputValue;

	static EditorObject* GetSelectedObject();
	static EditorObject* SelectObject(EditorObject* object);

	static bool isOverUI(vec2 point);

	static vec2 DrawProperty(vec2 drawPos, std::string name, std::string value, float sizeX);
	static vec2 DrawProperty(vec2 drawPos, std::string name, vec2 value, float sizeX);

	/// <summary>
	/// Displays a text input
	/// </summary>
	/// <param name="pos">The position of the text input</param>
	/// <param name="value">The currect value of the input</param>
	/// <param name="ID">An unique ID</param>
	/// <returns>A tuple containing:
	/// 1: the size of the text input
	/// 2: the new value when submitted by user, or the original value
	/// </returns>
	static std::tuple<vec2, std::string> TextInput(vec2 pos, std::string value, std::string ID);

private:
	static EditorObject* selectedObject;

	static void OnMainLoop();
	static void DrawPanel();
	static void OnClick(GLFWwindow* window, int button, int action, int mods);
};
