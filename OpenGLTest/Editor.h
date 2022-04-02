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
	const static int backspaceFirstLatency = 300; // milliseconds
	const static int backspaceLatency = 30; // milliseconds

	const static vec4 textColor;
	const static vec4 highlightColor;
	const static vec4 editColor;

	static std::string focusedTextInputID;
	static std::string focusedTextInputValue;
	static float backspaceNextTime;

	static EditorObject* GetSelectedObject();
	static EditorObject* SelectObject(EditorObject* object);

	static bool isOverUI(vec2 point);
	
	/// <summary>
	/// Displays a property
	/// </summary>
	/// <param name="drawPos">The position of property</param>
	/// <param name="name">The name displayed next to the textInput</param>
	/// <param name="value">The current value of the property</param>
	/// <param name="propX">The x offset of the textInput</param>
	/// <param name="ID">An unique ID</param>
	/// <returns>The size of the property</returns>
	static vec2 DrawProperty(vec2 drawPos, const std::string name, std::string* value, float propX, std::string ID);
	static vec2 DrawProperty(vec2 drawPos, const std::string name, float* value, float propX, std::string ID);
	static vec2 DrawProperty(vec2 drawPos, const std::string name, vec2* value, float propX, std::string ID);

	/// <summary>
	/// Displays a text input
	/// TODO: use a string builder, too lazy to use it
	/// </summary>
	/// <param name="pos">The position of the text input</param>
	/// <param name="value">The current value of the input</param>
	/// <param name="ID">An unique ID</param>
	/// <returns>The size of the text input</returns>
	static vec2 TextInput(vec2 pos, std::string* value, std::string ID, TextManager::text_align align = TextManager::right);

private:
	static EditorObject* selectedObject;

	static void OnMainLoop();
	static void DrawPanel();
	static void OnClick(GLFWwindow* window, int button, int action, int mods);
	static void OnCaracterInput(GLFWwindow* window, unsigned int codepoint);
};
