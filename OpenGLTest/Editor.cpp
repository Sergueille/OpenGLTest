#include "Editor.h"

#include "CircleCollider.h"
#include "Utility.h"

#include "Player.h"
#include "EditorSprite.h"

#include <iostream>

using namespace glm;

std::list<EditorObject*> Editor::editorObjects;
EditorObject* Editor::selectedObject;

bool Editor::enabled = false;

const float Editor::selectClickMargin = 0.001f;

const vec4 Editor::textColor = vec4(1);
const vec4 Editor::highlightColor = vec4(1, 0.5, 0.8, 1);
const vec4 Editor::editColor = vec4(1, 0.2, 0.5, 1);;

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

float Editor::moveSnapping = 0.2f;
float Editor::rotateSnapping = 5;
float Editor::sizeSnapping = 0.2f;

float Editor::rotateToolDegreesPerPixel = 1.3f;
float Editor::sizeToolSizePerPixel = 0.03f;

std::string Editor::currentFilePath = "";
MapData Editor::currentMapData = MapData();

std::list<std::string> Editor::panelWindows = {
	"Props",
	"Add",
	"Map",
	"File",
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

	EditorSaveManager::EnableEditorObjects();
	EventManager::Call(&EventManager::OnOpenEditor);
}

void Editor::CloseEditor()
{
	if (!enabled) return;
	enabled = false;

	EditorSaveManager::DisableEditorObjects();
	EventManager::Call(&EventManager::OnCloseEditor);
}

void Editor::DestroyEditor()
{
	std::cout << "Destroying editor" << std::endl;
	EditorSaveManager::ClearEditorLevel();
	CloseEditor();
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
	// click validation in OnClick() function;
	// keys for axis in OnKeyPressed()

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
		}
	}

	if (currentTool == Tool::none && !isOverUI(GetMousePos()))
	{
		editToolAxisVector = vec2(1, 1);

		// Change tool
		if (glfwGetKey(Utility::window, GLFW_KEY_G) == GLFW_PRESS)
		{
			currentTool = Tool::move;
			editToolStartMouse = mousePos;
			editToolStartPos = selectedObject->GetEditPos();
		}
		else if (glfwGetKey(Utility::window, GLFW_KEY_R) == GLFW_PRESS)
		{
			currentTool = Tool::rotate;
			editToolStartMouse = mousePos;
			editToolStartRot = selectedObject->GetEditRotation();
		}
		else if (glfwGetKey(Utility::window, GLFW_KEY_S) == GLFW_PRESS)
		{
			currentTool = Tool::scale;
			editToolStartMouse = mousePos;
			editToolStartScale = selectedObject->GetEditScale();
		}
	}
}

void Editor::HandleHotkeys(int key)
{
	// NOTE: shortcuts for transform edition are located in HandleTools()

	bool control = glfwGetKey(Utility::window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS
		|| glfwGetKey(Utility::window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

	bool alt = glfwGetKey(Utility::window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS
		|| glfwGetKey(Utility::window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;

	bool shift = glfwGetKey(Utility::window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
		|| glfwGetKey(Utility::window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

	
	if (control && alt && glfwGetKey(Utility::window, GLFW_KEY_S) == GLFW_PRESS && enabled) // Save As
	{
		currentPanelWindow = PanelWindow::file;
		focusedTextInputID = "FilePath";
	}
	else if (control && glfwGetKey(Utility::window, GLFW_KEY_S) == GLFW_PRESS && enabled) // Save
	{
		EditorSaveManager::SaveLevel();
	}
	else if (control && glfwGetKey(Utility::window, GLFW_KEY_O) == GLFW_PRESS && enabled) // Open
	{
		currentPanelWindow = PanelWindow::file;
		focusedTextInputID = "LoadPath";
	}
	else if (control && glfwGetKey(Utility::window, GLFW_KEY_N) == GLFW_PRESS && enabled) // New
	{
		EditorSaveManager::ClearEditorLevel();
		currentFilePath = "";
	}
	else if (glfwGetKey(Utility::window, GLFW_KEY_DELETE) == GLFW_PRESS && enabled) // Delete
	{
		if (selectedObject != nullptr)
			RemoveObject(selectedObject);
	}
	else if (control && glfwGetKey(Utility::window, GLFW_KEY_D) == GLFW_PRESS && enabled) // Duplicate
	{
		if (selectedObject != nullptr)
		{
			EditorObject* newObj = selectedObject->Copy();
			newObj->ID = Editor::IDmax;
			Editor::IDmax++;
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
		drawPos.y -= TextManager::RenderText("Object properties", drawPos, textSize).y + margin;
		drawPos.x += indentation;
		drawPos.y -= GetSelectedObject()->DrawProperties(drawPos).y + margin;
		drawPos.y -= GetSelectedObject()->DrawActions(drawPos).y;
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
		currentFilePath = loadPath;
		EditorSaveManager::LoadLevel(loadPath, true);
	}
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
		// End current tool
		currentTool = Tool::none;

		// Select an object
		if (!isOverUI(Utility::GetMousePos()))
		{
			vec2 worldPos = Utility::ScreenToWorld(Utility::GetMousePos());
			CircleCollider mouseColl = CircleCollider(worldPos, selectClickMargin);

			for (auto obj = editorObjects.begin(); obj != editorObjects.end(); obj++)
			{
				if ((*obj)->clickCollider == nullptr)
				{
					std::cout << "Editor object " << (*obj)->name << " has no collider and can't be clicked in the editor" << std::endl;
					continue;
				}

				vec3 res = (*obj)->clickCollider->CollideWith(&mouseColl);

				// Click on object
				if (res.z != 0)
				{
					selectedObject = *obj;
					break;
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
	return object;
}

void Editor::RemoveObject(EditorObject* object)
{
	if (GetSelectedObject()->ID == object->ID)
		SelectObject(nullptr);

	editorObjects.remove(object);

	delete object;
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

vec2 Editor::Button(vec3 drawPos, std::string text, bool* out, TextManager::text_align align)
{
	vec2 mousePos = Utility::GetMousePos();
	mousePos.y *= -1;
	mousePos.y += Utility::screenY;
	vec2 textRect = TextManager::GetRect(text, textSize);

	bool hoverX = mousePos.x > drawPos.x && mousePos.x < drawPos.x + textRect.x;
	bool hoverY = mousePos.y > drawPos.y && mousePos.y < drawPos.y + textRect.y;

	*out = false;

	vec4 color;
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

	TextManager::RenderText(text, drawPos, textSize, align, color);
	return textRect;
}
