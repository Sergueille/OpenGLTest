#include "Editor.h"

#include "CircleCollider.h"
#include "Utility.h"

#include "Player.h"
#include "EditorSprite.h"
#include "Laser.h"

#include <iostream>

using namespace glm;

std::list<EditorObject*> Editor::editorObjects;
EditorObject* Editor::selectedObject;

bool Editor::enabled = false;

const float Editor::selectClickMargin = 0.001f;

const vec4 Editor::textColor = vec4(1);
const vec4 Editor::highlightColor = vec4(1, 0.5, 0.8, 1);
const vec4 Editor::editColor = vec4(1, 0.2, 0.5, 1);;
const vec4 Editor::disabledTextColor = vec4(0.7, 0.7, 0.7, 0.7);

std::string Editor::focusedTextInputID = "";
std::string Editor::focusedTextInputValue = "";
float Editor::backspaceNextTime = 0;
float Editor::buttonAlreadyPressed = false;

int Editor::IDmax = 0;

Editor::Tool Editor::currentTool = Editor::Tool::none;
vec2 Editor::editToolStartMouse;
vec3 Editor::editToolStartPos;
float Editor::editToolStartRot;
vec2 Editor::editToolStartScale;
vec2 Editor::editToolAxisVector = vec2(1);
EditorObject* Editor::oldToolObject = nullptr;

float Editor::moveSnapping = 0.2f;
float Editor::rotateSnapping = 5;
float Editor::sizeSnapping = 0.2f;

float Editor::rotateToolDegreesPerPixel = 1.3f;
float Editor::sizeToolSizePerPixel = 0.03f;

std::string Editor::currentFilePath = "";
MapData Editor::currentMapData = MapData();

std::stack<Editor::UndoAction> Editor::undoStack;
std::stack<Editor::UndoAction> Editor::redoStack;

bool Editor::propertyChanged = false;

std::list<std::string> Editor::panelWindows = {
	"Props",
	"Add",
	"Map",
	"File",
	"Settings",
};
Editor::PanelWindow Editor::currentPanelWindow = Editor::PanelWindow::properties;

void Editor::CreateEditor()
{
	std::cout << "Creating editor" << std::endl;

	EventManager::OnMainLoop.push_end(OnMainLoop);
	EventManager::OnClick.push_end(OnClick);
	EventManager::OnCharPressed.push_end(OnCaracterInput);
	EventManager::OnKeyPressed.push_end(OnKeyPressed);

	OpenEditor();
}

void Editor::OpenEditor()
{
	if (enabled) return;
	enabled = true;

	Camera::editorCamera = true;
	EditorSaveManager::EnableEditorObjects();
	EventManager::Call(&EventManager::OnOpenEditor);
}

void Editor::CloseEditor()
{
	if (!enabled) return;
	enabled = false;

	Camera::editorCamera = false;
	EditorSaveManager::DisableEditorObjects();
	EventManager::Call(&EventManager::OnCloseEditor);
}

void Editor::DestroyEditor()
{
	std::cout << "Destroying editor" << std::endl;
	EditorSaveManager::ClearEditorLevel();
	CloseEditor();
}

int Editor::GetIndexOfEditorObject(EditorObject* object, bool throwIfNotFound)
{
	int index = 0;
	for (auto it = editorObjects.begin(); it != editorObjects.end(); it++)
	{
		if ((*it)->ID == object->ID)
			return index;

		index++;
	}

	if (throwIfNotFound)
		throw "Editor object not found in list";

	return -1;
}

void Editor::Undo()
{
	if (undoStack.size() < 1)
	{
		std::cout << "Undo stack is empty" << std::endl;
		return;
	}

	UndoAction topAction = undoStack.top();
	undoStack.pop();
	std::cout << "Undo: " << topAction.GetDescription() << std::endl;

	topAction.Do();
	redoStack.push(topAction.GetOpposite());
}

void Editor::Redo()
{
	if (redoStack.size() < 1)
	{
		std::cout << "Redo stack is empty" << std::endl;
		return;
	}

	UndoAction topAction = redoStack.top();
	redoStack.pop();
	std::cout << "Redo: " << topAction.GetDescription() << std::endl;

	topAction.Do();
	undoStack.push(topAction.GetOpposite());
}

void Editor::OnMainLoop()
{
	if (!enabled) // EDITOR ONLY
		return;

	HandleTools();
	DrawPanel();
	HandleInputBackspace();
}

void Editor::HandleTools()
{
	// The tool handling is scattered in the Edior code:
	//		keys to enable tools are in HandleHotkeys()
	//		click for validation is in OnClick() function
	//		keys for axis are in OnKeyPressed()
	//		can change tool with SelectTool()

	if (selectedObject == nullptr)
		return;

	vec2 mousePos = GetMousePos();
	vec2 worldMousePos = ScreenToWorld(mousePos);
	bool snapping = glfwGetKey(Utility::window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE;

	if (currentTool == Tool::move) // MOVE
	{
		vec2 pos = (worldMousePos - vec2(editToolStartPos)) * editToolAxisVector + vec2(editToolStartPos);
		if (snapping)
		{
			pos.x = round(pos.x / moveSnapping) * moveSnapping;
			pos.y = round(pos.y / moveSnapping) * moveSnapping;
		}
		selectedObject->SetEditPos(vec3(pos.x, pos.y, editToolStartPos.z));

		if (glfwGetKey(Utility::window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // Cancel
		{
			currentTool = Tool::none;
			selectedObject->SetEditPos(editToolStartPos);

			if (oldToolObject != nullptr)
			{
				delete oldToolObject;
				oldToolObject = nullptr;
			}
		}
	}
	else if (currentTool == Tool::rotate) // ROTATE
	{
		float angle = -(mousePos.x - editToolStartMouse.x) * rotateToolDegreesPerPixel;
		if (snapping) angle = round(angle / rotateSnapping) * rotateSnapping;
		selectedObject->SetEditRotation(editToolStartRot + angle);

		if (glfwGetKey(Utility::window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // Cancel
		{
			currentTool = Tool::none;
			selectedObject->SetEditRotation(editToolStartRot);

			if (oldToolObject != nullptr)
			{
				delete oldToolObject;
				oldToolObject = nullptr;
			}
		}
	}
	else if (currentTool == Tool::scale) // SCALE
	{
		vec2 scale = (((mousePos.x - editToolStartMouse.x) * sizeToolSizePerPixel) + 1) * editToolStartScale;
		if (snapping)
		{
			scale.x = round(scale.x / sizeSnapping) * sizeSnapping;
			scale.y = round(scale.y / sizeSnapping) * sizeSnapping;
		}
		selectedObject->SetEditScale(scale * editToolAxisVector + editToolStartScale * (vec2(1) - editToolAxisVector));

		if (glfwGetKey(Utility::window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // Cancel
		{
			currentTool = Tool::none;
			selectedObject->SetEditScale(editToolStartScale);

			if (oldToolObject != nullptr)
			{
				delete oldToolObject;
				oldToolObject = nullptr;
			}
		}
	}
}

void Editor::SelectTool(Tool tool)
{
	if (selectedObject == nullptr) return;
	currentTool = tool;
	editToolStartMouse = GetMousePos();
	editToolStartPos = selectedObject->GetEditPos();
	editToolStartRot = selectedObject->GetEditRotation();
	editToolStartScale = selectedObject->GetEditScale();
	editToolAxisVector = vec2(1, 1);

	if (oldToolObject != nullptr)
	{
		delete oldToolObject;
		oldToolObject = nullptr;
	}

	oldToolObject = selectedObject->Copy();
	oldToolObject->Disable();
}

void Editor::HandleHotkeys(int key)
{
	bool control = glfwGetKey(Utility::window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
		|| glfwGetKey(Utility::window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

	bool alt = glfwGetKey(Utility::window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS
		|| glfwGetKey(Utility::window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;

	bool shift = glfwGetKey(Utility::window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
		|| glfwGetKey(Utility::window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

	
	if (enabled && control && alt && glfwGetKey(Utility::window, GLFW_KEY_S) == GLFW_PRESS) // Save As
	{
		currentPanelWindow = PanelWindow::file;
		focusedTextInputID = "FilePath";
	}
	else if (enabled && control && glfwGetKey(Utility::window, GLFW_KEY_S) == GLFW_PRESS) // Save
	{
		EditorSaveManager::SaveLevel();
	}
	else if (enabled && control && glfwGetKey(Utility::window, GLFW_KEY_O) == GLFW_PRESS) // Open
	{
		currentPanelWindow = PanelWindow::file;
		focusedTextInputID = "LoadPath";
	}
	else if (enabled && control && glfwGetKey(Utility::window, GLFW_KEY_N) == GLFW_PRESS) // New
	{
		EditorSaveManager::ClearEditorLevel();
		currentFilePath = "";
	}
	else if (enabled && glfwGetKey(Utility::window, GLFW_KEY_DELETE) == GLFW_PRESS) // Delete
	{
		if (selectedObject != nullptr)
			RemoveObject(selectedObject);
	}
	else if (enabled && control && glfwGetKey(Utility::window, GLFW_KEY_D) == GLFW_PRESS) // Duplicate
	{
		if (selectedObject != nullptr)
		{
			EditorObject* newObj = selectedObject->Copy();
			newObj->ID = Editor::IDmax;
			Editor::IDmax++;
			Editor::AddObject(newObj);
			Editor::SelectObject(newObj);
		}
	}
	else if (control && glfwGetKey(Utility::window, GLFW_KEY_T) == GLFW_PRESS) // Test level
	{
		if (enabled) // Start test
			StartTest();
		else // End test
			EndTest();
	}
	else if (enabled && control && shift && glfwGetKey(Utility::window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Redo();
	}
	else if (enabled && control && glfwGetKey(Utility::window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Undo();
	}
	else if (enabled && control && glfwGetKey(Utility::window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		Redo();
	}

	// Tools shortcuts
	if (currentTool == Tool::none && !isOverUI(GetMousePos()) && enabled && !control && !shift && !alt && focusedTextInputID == "")
	{
		// Change tool
		if (glfwGetKey(Utility::window, GLFW_KEY_G) == GLFW_PRESS)
		{
			SelectTool(Tool::move);
		}
		else if (glfwGetKey(Utility::window, GLFW_KEY_R) == GLFW_PRESS)
		{
			SelectTool(Tool::rotate);
		}
		else if (glfwGetKey(Utility::window, GLFW_KEY_S) == GLFW_PRESS)
		{
			SelectTool(Tool::scale);
		}
	}
}

void Editor::DrawPanel()
{
	// Start position: to left
	vec3 drawPos = vec3(10, Utility::screenY - textSize - margin, UIBaseZPos + 5);

	// Background
	Sprite(vec3(0, 0, UIBaseZPos), vec3(panelSize, Utility::screenY, UIBaseZPos), vec4(0, 0, 0, 0.3f)).Draw();

	// Draw tabs buttons
	int i = 0;
	for (auto name = panelWindows.begin(); name != panelWindows.end(); name++)
	{
		bool pressed;
		drawPos.x += Button(drawPos, *name, &pressed).x + margin;

		if (pressed)
			currentPanelWindow = (PanelWindow)i;

		i++;
	}

	drawPos.x = 10;
	drawPos.y -= textSize + margin;

	switch (currentPanelWindow)
	{
	case PanelWindow::properties:
		DrawPropsTab(drawPos);
		break;
	case PanelWindow::add:
		DrawAddTab(drawPos);
		break;
	case PanelWindow::map:
		DrawMapTab(drawPos);
		break;
	case PanelWindow::file:
		DrawFileTab(drawPos);
		break;
	case PanelWindow::settings:
		DrawSettingsTab(drawPos);
		break;
	default:
		throw "Selected panel tab id is too high!";
		break;
	}
}

void Editor::DrawPropsTab(vec3 drawPos)
{
	if (GetSelectedObject() == nullptr)
	{
		TextManager::RenderText("No selected object", drawPos, textSize);
	}
	else
	{
		propertyChanged = false;
		EditorObject* copy = GetSelectedObject()->Copy();
		copy->Disable();

		drawPos.y -= TextManager::RenderText("Object properties", drawPos, textSize).y + margin;
		drawPos.x += indentation;
		drawPos.y -= GetSelectedObject()->DrawProperties(drawPos).y + margin;
		drawPos.y -= GetSelectedObject()->DrawActions(drawPos).y;

		if (propertyChanged)
		{
			EditorObject* newCopy = GetSelectedObject()->Copy();
			newCopy->Disable();
			RecordObjectChange(copy, newCopy);
		}
		else
		{
			delete copy;
			copy = nullptr;
		}
	}
}

void Editor::DrawAddTab(vec3 drawPos)
{
	drawPos.y -= TextManager::RenderText("Add an object", drawPos, textSize).y + margin;
	drawPos.x += indentation;

	bool pressed = false;
	EditorObject* newObject = nullptr;

	vec3 newPos = vec3(Camera::position.x, Camera::position.y, 0);

	drawPos.y -= Button(drawPos, "Sprite", &pressed).y;
	if (pressed)
		newObject = (EditorObject*)new EditorSprite(newPos);

	drawPos.y -= Button(drawPos, "Player", &pressed).y;
	if (pressed)
		newObject = (EditorObject*)new Player(newPos);

	drawPos.y -= Button(drawPos, "Laser", &pressed).y;
	if (pressed)
		newObject = (EditorObject*)new Laser();

	if (newObject != nullptr)
	{
		AddObject(newObject);
		selectedObject = newObject;
	}
}

void Editor::DrawMapTab(vec3 drawPos)
{
	drawPos.y -= TextManager::RenderText("Map properties", drawPos, textSize).y + margin;
	drawPos.x += indentation;

	drawPos.y -= DrawProperty(drawPos, "Map name", &currentMapData.mapName, panelPropertiesX, "MapName").y;
}

void Editor::DrawFileTab(vec3 drawPos)
{
	drawPos.y -= TextManager::RenderText("File", drawPos, textSize).y + margin;
	drawPos.x += indentation;

	drawPos.y -= DrawProperty(drawPos, "File path", &currentFilePath, panelPropertiesX, "FilePath").y;

	bool res;
	drawPos.y -= Button(drawPos, "Save!", &res).y + margin;
	if (res) EditorSaveManager::SaveLevel();

	std::string loadPath = "";
	drawPos.y -= DrawProperty(drawPos, "Load a level", &loadPath, panelPropertiesX, "LoadPath").y + margin;
	if (loadPath != "")
	{
		EditorSaveManager::LoadLevel(loadPath, true);
	}
}

void Editor::DrawSettingsTab(vec3 drawPos)
{
	drawPos.y -= TextManager::RenderText("Editor settings\nThese props are not saved for now", drawPos, textSize).y + margin;
	drawPos.x += indentation;

	drawPos.y -= DrawProperty(drawPos, "Move tool step", &moveSnapping, panelPropertiesX, "moveSnap").y;
	drawPos.y -= DrawProperty(drawPos, "Rotate tool step", &rotateSnapping, panelPropertiesX, "rotateSnap").y;
	drawPos.y -= DrawProperty(drawPos, "Scale tool step", &sizeSnapping, panelPropertiesX, "scaleSnap").y;
}

void Editor::HandleInputBackspace()
{
	if (focusedTextInputID != "")
	{
		if (glfwGetKey(Utility::window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		{
			// Control: remove word
			if (glfwGetKey(Utility::window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(Utility::window, GLFW_KEY_RIGHT_CONTROL))
			{
				if (backspaceNextTime < Utility::time)
				{
					if (focusedTextInputValue.length() > 0)
					{
						focusedTextInputValue.pop_back();
					}

					while (focusedTextInputValue.length() > 0 && isalpha(focusedTextInputValue[focusedTextInputValue.length() - 1]))
					{
						focusedTextInputValue.pop_back();
					}

					backspaceNextTime = Utility::time + (backspaceFirstLatency / 1000.f);
				}
			}
			else
			{
				// remove last letter
				if (focusedTextInputValue.length() > 0 && backspaceNextTime < Utility::time)
				{
					focusedTextInputValue.pop_back();
					if (backspaceNextTime == 0)
						backspaceNextTime = Utility::time + (backspaceFirstLatency / 1000.f);
					else
						backspaceNextTime = Utility::time + (backspaceLatency / 1000.f);
				}
			}
		}
		else
		{
			backspaceNextTime = 0;
		}
	}
}

void Editor::OnClick(GLFWwindow* window, int button, int action, int mods)
{
	if (!enabled)
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (currentTool != Tool::none)
		{
			// End current tool
			currentTool = Tool::none;
			EditorObject* newObject = selectedObject->Copy();
			newObject->Disable();
			RecordObjectChange(oldToolObject, newObject);
			oldToolObject = nullptr;
			return; // Ignore other clic events
		}

		// Select an object
		if (!isOverUI(Utility::GetMousePos())) // If the mouse is not over the UI
		{
			vec2 worldPos = Utility::ScreenToWorld(Utility::GetMousePos()); // Mouse pos in world space
			CircleCollider mouseColl = CircleCollider(worldPos, selectClickMargin, false); // Create a small collider for the mouse

			float maxZ = -FLT_MAX; // Max z pos found

			// For each object
			for (auto obj = editorObjects.begin(); obj != editorObjects.end(); obj++)
			{
				if ((*obj)->clickCollider == nullptr) // No click collider: that's a problem
				{
					std::cout << "Editor object " << (*obj)->name << " has no collider and can't be clicked in the editor" << std::endl;
					continue;
				}

				if ((*obj)->GetEditPos().z > maxZ) // If the z is high enough
				{
					// Result of the collision
					vec3 res = (*obj)->clickCollider->CollideWith(&mouseColl);

					// If collision, select
					if (res.z != 0)
					{
						selectedObject = *obj;
						maxZ = (*obj)->GetEditPos().z;
					}
				}
			}
		}
	}
}

void Editor::OnCaracterInput(GLFWwindow* window, unsigned int codepoint)
{
	if (!enabled)
		return;

	if (focusedTextInputID != "")
	{
		focusedTextInputValue += char(codepoint);
	}
}

vec2 Editor::TextInput(vec3 pos, std::string* value, std::string ID, TextManager::text_align align)
{
	if (focusedTextInputID == ID)
	{
		if (glfwGetKey(Utility::window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			*value = focusedTextInputValue;
			focusedTextInputID = "";
			focusedTextInputValue = "";
			propertyChanged = true;
		}
		else if (glfwGetKey(Utility::window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			focusedTextInputID = "";
		}

		bool showCursor = float(static_cast<int>(Utility::time * 1000) % textCursorBlink) > (textCursorBlink / 2.f);
		std::string displayString = showCursor ? focusedTextInputValue + "|" : focusedTextInputValue;
		vec2 textRect = TextManager::RenderText(displayString, pos, textSize, align, editColor);
		return textRect;
	}
	else
	{
		std::string displayString = value->length() == 0 ? "<empty>" : *value;

		vec2 mousePos = Utility::GetMousePos();
		mousePos.y *= -1;
		mousePos.y += Utility::screenY;
		vec2 textRect = TextManager::GetRect(displayString, textSize);

		bool hoverX = mousePos.x > pos.x && mousePos.x < pos.x + textRect.x;
		bool hoverY = mousePos.y > pos.y && mousePos.y < pos.y + textRect.y;
		
		vec4 color;
		if (hoverX && hoverY)
		{
			if (glfwGetMouseButton(Utility::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				focusedTextInputID = ID;
				focusedTextInputValue = "";
			}

			float lerpValue = Utility::GetTimeSine(hoverHighlightDuration);
			color = highlightColor + (textColor - highlightColor) * lerpValue;
		}
		else
		{
			color = textColor;
		}

		TextManager::RenderText(displayString, pos, textSize, align, color);
		return textRect;
	}
}

EditorObject* Editor::GetSelectedObject()
{
	return selectedObject;
}

EditorObject* Editor::SelectObject(EditorObject* object)
{
	selectedObject = object;
	return selectedObject;
}

EditorObject* Editor::AddObject(EditorObject* object)
{
	editorObjects.push_back(object);

	// Record undo entry
	UndoAction action = UndoAction{ UndoAction::UndoActType::remove };
	action.object = nullptr;
	action.otherObject = object;
	action.index = GetIndexOfEditorObject(object, true);
	undoStack.push(action);
	ClearRedoStack();

	return object;
}

void Editor::RemoveObject(EditorObject* object)
{
	// Record undo entry
	UndoAction action = UndoAction{ UndoAction::UndoActType::add };
	action.object = object;
	action.otherObject = nullptr;
	action.index = GetIndexOfEditorObject(object, true);
	undoStack.push(action);
	ClearRedoStack();

	if (GetSelectedObject()->ID == object->ID)
		SelectObject(nullptr);

	editorObjects.remove(object);

	object->Disable();
}

void Editor::StartTest()
{
	if (currentFilePath.length() < 1)
	{
		std::cout << "No file path specified!" << std::endl;
		currentPanelWindow = PanelWindow::file;
		focusedTextInputID = "filePath";
	}

	EditorSaveManager::SaveLevel();
	CloseEditor();
	EditorSaveManager::LoadLevel(currentFilePath, false);
}

void Editor::EndTest()
{
	EditorSaveManager::ClearGameLevel();
	OpenEditor();
}

bool Editor::isOverUI(vec2 point)
{
	return point.x < panelSize;
}

vec2 Editor::OprionProp(vec3 startPos, std::string name, int* value, int max, std::string* firstDisplay, float propX)
{
	vec3 drawPos = startPos;

	TextManager::RenderText(name + ":", drawPos, textSize);
	drawPos.x += propX;

	bool pressed;

	drawPos.x += Button(drawPos, "(-)", &pressed, *value > 0).x + margin;
	if (pressed)
		(*value)--;

	drawPos.x += TextManager::RenderText(*(firstDisplay + *value), drawPos, textSize).x + margin;

	vec2 btnSize = Button(drawPos, "(+)", &pressed, *value < max);
	drawPos += vec3(btnSize.x, -btnSize.y, 0);
	if (pressed)
		(*value)++;

	return vec2(startPos - drawPos);
}

vec2 Editor::DrawProperty(vec3 drawPos, const std::string name, std::string* value, float propX, std::string ID)
{
	TextManager::RenderText(name + ":", drawPos, textSize);
	drawPos.x += propX;
	vec2 propSize = TextInput(drawPos, value, ID);

	return vec2(propX + propSize.x, textSize);
}

vec2 Editor::DrawProperty(vec3 drawPos, const std::string name, float* value, float propX, std::string ID)
{
	std::string res = std::to_string(*value);
	vec2 size = DrawProperty(drawPos, name, &res, propX, ID);

	try { *value = std::stof(res); }
	catch (std::exception) { *value = 0.f; }

	return size;
}

vec2 Editor::DrawProperty(vec3 drawPos, const std::string name, int* value, float propX, std::string ID)
{
	std::string res = std::to_string(*value);
	vec2 size = DrawProperty(drawPos, name, &res, propX, ID);

	try { *value = std::stoi(res); }
	catch (std::exception) { *value = 0; }

	return size;
}

vec2 Editor::DrawProperty(vec3 drawPos, const std::string name, vec2* value, float propX, std::string ID)
{
	vec4 realValue = vec4(value->x, value->y, 0, 0);
	vec2 size = DrawProperty(drawPos, name, &realValue, 2, propX, ID);
	*value = vec2(realValue);
	return size;
}

vec2 Editor::DrawProperty(vec3 drawPos, const std::string name, vec3* value, float propX, std::string ID, bool colorNames)
{
	vec4 realValue = vec4(value->x, value->y, value->z, 0);
	vec2 size = DrawProperty(drawPos, name, &realValue, 3, propX, ID, colorNames);
	*value = vec3(realValue);
	return size;
}

vec2 Editor::DrawProperty(vec3 drawPos, const std::string name, vec4* value, float propX, std::string ID, bool colorNames)
{
	return DrawProperty(drawPos, name, value, 4, propX, ID, colorNames);
}

vec2 Editor::DrawProperty(vec3 drawPos, const std::string name, vec4* value, int numComponents, float propX, std::string ID, bool colorNames)
{
	vec2 startPos = drawPos;

	std::string resX = std::to_string(value->x);
	std::string resY = std::to_string(value->y);
	std::string resZ = std::to_string(value->z);
	std::string resW = std::to_string(value->w);

	drawPos.y -= TextManager::RenderText(name + ":", drawPos, textSize).y;
	drawPos.x += indentation;

	std::string names[4] = {
		colorNames ? "Red" : "X",
		colorNames ? "Green" : "Y",
		colorNames ? "Blue" : "Z",
		colorNames ? "Alpha" : "W",
	};

	vec2 sizeX, sizeY, sizeZ, sizeW = vec2(0);
	if (numComponents > 0)
		sizeX = DrawProperty(drawPos, names[0], &resX, propX - indentation, ID + "X");
	drawPos.y -= sizeX.y;
	if (numComponents > 1)
		sizeY = DrawProperty(drawPos, names[1], &resY, propX - indentation, ID + "Y");
	drawPos.y -= sizeY.y;
	if (numComponents > 2)
		sizeZ = DrawProperty(drawPos, names[2], &resZ, propX - indentation, ID + "Z");
	drawPos.y -= sizeZ.y;
	if (numComponents > 3)
		sizeW = DrawProperty(drawPos, names[3], &resW, propX - indentation, ID + "W");
	drawPos.y -= sizeW.y;

	float maxX = max(max(sizeX.x, sizeY.x), max(sizeZ.x, sizeW.x));
	drawPos.x = propX + maxX;

	try { value->x = std::stof(resX); }
	catch (std::exception) { value->x = 0.f; }

	try { value->y = std::stof(resY); }
	catch (std::exception) { value->y = 0.f; }

	try { value->z = std::stof(resZ); }
	catch (std::exception) { value->z = 0.f; }

	try { value->w = std::stof(resW); }
	catch (std::exception) { value->w = 0.f; }

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

void Editor::OnKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		HandleHotkeys(key);

	// Switch tool axis
	if (currentTool != Tool::none)
	{
		if (key == GLFW_KEY_X && action == GLFW_PRESS)
		{
			if (editToolAxisVector == vec2(1, 0))
				editToolAxisVector = vec2(1, 1);
			else
				editToolAxisVector = vec2(1, 0);
		}
		else if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		{
			if (editToolAxisVector == vec2(0, 1))
				editToolAxisVector = vec2(1, 1);
			else
				editToolAxisVector = vec2(0, 1);
		}
	}
}

vec2 Editor::Button(vec3 drawPos, std::string text, bool* out, bool enabled, TextManager::text_align align)
{
	vec4 color;
	vec2 textRect = TextManager::GetRect(text, textSize);

	if (!enabled)
	{
		color = disabledTextColor;
		*out = false;
	}
	else
	{
		vec2 mousePos = Utility::GetMousePos();
		mousePos.y *= -1;
		mousePos.y += Utility::screenY;

		bool hoverX = mousePos.x > drawPos.x && mousePos.x < drawPos.x + textRect.x;
		bool hoverY = mousePos.y > drawPos.y && mousePos.y < drawPos.y + textRect.y;

		*out = false;

		if (hoverX && hoverY)
		{
			if (glfwGetMouseButton(Utility::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				if (!buttonAlreadyPressed)
				{
					*out = true;
					color = editColor;
					buttonAlreadyPressed = true;
				}
			}
			else
			{
				float lerpValue = Utility::GetTimeSine(hoverHighlightDuration);
				color = highlightColor + (textColor - highlightColor) * lerpValue;
				buttonAlreadyPressed = false;
			}
		}
		else
		{
			color = textColor;
		}
	}

	TextManager::RenderText(text, drawPos, textSize, align, color);
	return textRect;
}

vec2 Editor::CheckBox(vec3 drawPos, std::string label, bool* value, float textWidth)
{
	vec2 startPos = vec2(drawPos);
	std::string displayString = *value ? "Yes" : "No";
	bool textClick, boxClick;
	Button(drawPos, label, &textClick);
	drawPos.x += textWidth;
	vec2 btnSize = Button(drawPos, displayString, &boxClick);
	drawPos += vec3(btnSize.x, btnSize.y, 0);

	if (textClick || boxClick)
	{
		*value = !(*value);
		propertyChanged = true;
	}

	return vec2(drawPos) - startPos;
}

void Editor::RecordObjectChange(EditorObject* oldObject, EditorObject* newObject)
{
	UndoAction action = UndoAction{ UndoAction::UndoActType::change };
	action.object = oldObject;
	action.otherObject = newObject;
	action.index = GetIndexOfEditorObject(newObject, true);
	undoStack.push(action);
	ClearRedoStack();
}

void Editor::ClearUndoStack()
{
	while (!undoStack.empty())
	{
		delete undoStack.top().object;
		undoStack.pop();
	}
}

void Editor::ClearRedoStack()
{
	while (!redoStack.empty())
	{
		delete redoStack.top().object;
		redoStack.pop();
	}
}

void Editor::UndoAction::Do()
{
	auto iter = index == 0 ? editorObjects.begin() : std::next(editorObjects.begin(), index);

	if (type == UndoActType::add)
	{
		EditorObject* copy = object->Copy();
		editorObjects.insert(iter, copy);
		copy->Enable();
	}
	else if (type == UndoActType::remove)
	{
		if (GetSelectedObject() != nullptr && GetSelectedObject()->ID == (*iter)->ID)
			SelectObject(nullptr);
		(*iter)->Disable();
		editorObjects.erase(iter);
	}
	else if (type == UndoActType::change)
	{
		(*iter)->Disable();
		auto newIter = editorObjects.erase(iter);
		EditorObject* copy = object->Copy();
		editorObjects.insert(newIter, copy);
		copy->Enable();
	}
}

Editor::UndoAction Editor::UndoAction::GetOpposite()
{
	UndoAction newAction = {};

	if (this->type == UndoActType::add)
	{
		newAction.type = UndoActType::remove;
	}
	else if (this->type == UndoActType::remove)
	{
		newAction.type = UndoActType::add;
	}
	else if (this->type == UndoActType::change)
	{
		newAction.type = UndoActType::change;
	}

	newAction.object = this->otherObject;
	newAction.otherObject = this->object;

	return newAction;
}

std::string Editor::UndoAction::GetDescription()
{
	if (type == UndoActType::add)
	{
		return "Add object at index " + std::to_string(index);
	}
	else if (type == UndoActType::remove)
	{
		return "Remove object at index " + std::to_string(index);
	}
	else if (type == UndoActType::change)
	{
		return "Change object at index " + std::to_string(index);
	}
}
