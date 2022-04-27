#pragma once

#include "EventManager.h"
#include "TextManager.h"
#include "Camera.h"
#include "EditorSaveManager.h"

#include <GLFW/glfw3.h>
#include <list>
#include <functional>
#include <stack>

using namespace glm;

class EditorObject;
struct MapData;
struct EditorAction;
class EditorSaveManager;
class Editor
{
public:
	enum class Tool
	{
		none, move, rotate, scale
	};

	enum class PanelWindow
	{
		properties, add, map, file, settings
	};

	struct UndoAction {
		enum class UndoActType { add, change, remove };
		UndoActType type;
		EditorObject* object = nullptr;
		EditorObject* otherObject = nullptr;
		int index;

		void Do();
		UndoAction GetOpposite();
		std::string GetDescription();
	};

	static EditorAction editorActions[14];

	static std::list<EditorObject*> editorObjects;

	static bool enabled;

	const static int panelSize = 400;
	const static int panelPropertiesX = 150;
	const static int infoBarWidth = 20;

	const static int textSize = 15;
	const static int margin = 10;
	const static int indentation = 10;

	const static float selectClickMargin;

	const static int hoverHighlightDuration = 500; // milliseconds
	const static int textCursorBlink = 400; // milliseconds
	const static int backspaceFirstLatency = 300; // milliseconds
	const static int backspaceLatency = 30; // milliseconds

	const static int UIBaseZPos = 100; // The Z position of the UI elements to use

	static float rotateToolDegreesPerPixel; // Rotration speed for rotate tool
	static float sizeToolSizePerPixel; // Rotration speed for rotate tool

	static float moveSnapping;
	static float rotateSnapping;
	static float sizeSnapping;

	const static vec4 textColor;
	const static vec4 highlightColor;
	const static vec4 editColor;
	const static vec4 disabledTextColor;
	const static vec4 backgroundColor;
	const static vec4 infobarColor;

	static std::string focusedTextInputID;
	static std::string focusedTextInputValue;
	static float backspaceNextTime;
	static float buttonAlreadyPressed;

	/// <summary>
	/// Where to save the map, relative to levelsBasePath
	/// </summary>
	static std::string currentFilePath;

	static MapData currentMapData;

	/// <summary>
	/// Names of the tabs of the panel
	/// </summary>
	static std::list<std::string> panelWindows;
	/// <summary>
	/// Panel tab that is currently selected
	/// </summary>
	static PanelWindow currentPanelWindow;

	/// <summary>
	/// Max usique object id attribued, used to create new ones
	/// </summary>
	static int IDmax;

	static Tool currentTool;

	static std::stack<UndoAction> undoStack;
	static std::stack<UndoAction> redoStack;

	static std::string infoBarText;

	static void CreateEditor(); // Setup editor and open it
	static void OpenEditor(); // Open the editor, must be set up before
	static void CloseEditor(); // Close the editor but don't destroy it
	static void DestroyEditor(); // Destroy all editor

	static void StartTest();
	static void EndTest();

	static EditorObject* GetSelectedObject();
	static std::list<EditorObject*>* GetAllSelectedObjects();
	static EditorObject* SelectObject(EditorObject* object, bool addToCurrentSelection = false);
	static std::list<EditorObject*>* SelectObjects(std::list<EditorObject*> objects);

	static EditorObject* AddObject(EditorObject* object);
	static void RemoveObject(EditorObject* object);

	/// <summary>
	/// Is the point in screen coordinates over editor UI?
	/// TODO: check if working
	/// </summary>
	static bool isOverUI(vec2 point);
	
	/// <summary>
	/// Displays an object property
	/// </summary>
	/// <param name="drawPos">The position of property</param>
	/// <param name="name">The name displayed next to the textInput</param>
	/// <param name="value">The current value of the property</param>
	/// <param name="propX">The x offset of the textInput</param>
	/// <param name="ID">An unique ID</param>
	/// <returns>The size of the property</returns>
	static vec2 DrawProperty(vec3 drawPos, const std::string name, std::string* value, float propX, std::string ID);
	static vec2 DrawProperty(vec3 drawPos, const std::string name, float* value, float propX, std::string ID);
	static vec2 DrawProperty(vec3 drawPos, const std::string name, int* value, float propX, std::string ID);
	static vec2 DrawProperty(vec3 drawPos, const std::string name, vec2* value, float propX, std::string ID);
	static vec2 DrawProperty(vec3 drawPos, const std::string name, vec3* value, float propX, std::string ID, bool colorNames = false);
	static vec2 DrawProperty(vec3 drawPos, const std::string name, vec4* value, float propX, std::string ID, bool colorNames = false);

	/// <summary>
	/// <para>Displays a text input</para>
	/// <para>TODO: use a string builder, too lazy to use it</para>
	/// </summary>
	/// <param name="pos">The position of the text input</param>
	/// <param name="value">The current value of the input</param>
	/// <param name="ID">An unique ID</param>
	/// <param name="needReturn">If true (by default) the user need to press the return key to send the value</param>
	/// <returns>The size of the text input</returns>
	static vec2 TextInput(vec3 pos, std::string* value, std::string ID, TextManager::text_align align = TextManager::right, bool needReturn = true);

	static vec2 Button(vec3 drawPos, std::string text, bool* out, bool enabled = true, TextManager::text_align align = TextManager::right);

	static vec2 CheckBox(vec3 drawPos, std::string label, bool* value, float textWidth);

	static vec2 OprionProp(vec3 drawPos, std::string name, int* value, int max, std::string* firstDisplay, float propX);

	static int GetIndexOfEditorObject(EditorObject* object, bool throwIfNotFound);

	// TODO: Redo system not working becaus of object references
	static void Undo();
	static void Redo();
	static void ClearUndoStack();
	static void ClearRedoStack();

	static EditorObject* GetObjectUnderMouse();

private:
	static std::list<EditorObject*> selectedObjects;

	static vec2 editToolStartMouse;
	static std::list<vec3> editToolStartPos;
	static std::list<float> editToolStartRot;
	static std::list<vec2> editToolStartScale;
	static vec2 editToolAxisVector; // Used to apply tools only on ona axis (ex: press G X)
	static EditorObject* oldToolObject;

	/// <summary>
	/// Will become true if a text input or a checkbox is changed
	/// </summary>
	static bool propertyChanged;

	static void OnMainLoop();

	static void DrawPanel();
	static void DrawPropsTab(vec3 drawPos);
	static void DrawAddTab(vec3 drawPos);
	static void DrawMapTab(vec3 drawPos);
	static void DrawFileTab(vec3 drawPos);
	static void DrawSettingsTab(vec3 drawPos);

	static void DrawInfoBar();

	static void HandleInputBackspace();
	static void HandleTools();
	/// <summary>
	/// Called by OnKeyPressed()
	/// </summary>
	static void HandleHotkeys(int key);

	static void SelectTool(Tool tool);

	static void OnClick(GLFWwindow* window, int button, int action, int mods);
	static void OnCaracterInput(GLFWwindow* window, unsigned int codepoint);
	static void OnKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);

	static vec2 DrawProperty(vec3 drawPos, const std::string name, vec4* value, int numComponents, float propX, std::string ID, bool colorNames = false);

	/// <summary>
	/// Call this method after making a change to en entity, used for the undo system <br/>
	/// Provide COPIES of the obects
	/// </summary>
	static void RecordObjectChange(EditorObject* oldObject, EditorObject* newObject);
};

struct MapData
{
	std::string mapName = "";
};

/// <summary>
/// Action that can be done with a key shortcut or through the serch bar
/// </summary>
struct EditorAction
{
	std::string actionName;
	std::string description;

	int shortcutKey;
	bool needControl;
	bool needAlt;
	bool needShift;

	std::function<void()> func;

	bool needEnabled = true;

	std::string GetHoykeyDesc();
};
