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

const float Editor::selectClickMargin = 0.1f;

const vec4 Editor::textColor = vec4(1);
const vec4 Editor::highlightColor = vec4(1, 0.5, 0.8, 1);
const vec4 Editor::editColor = vec4(1, 0.2, 0.5, 1);;

std::string Editor::focusedTextInputID = "";
std::string Editor::focusedTextInputValue = "";
float Editor::backspaceNextTime = 0;
float Editor::buttonAlreadyPressed = false;

int Editor::IDmax = 0;

std::list<std::string> Editor::panelWindows = {
	"Properties",
	"Add",
	"Map info",
};
int Editor::currentPanelWindow = 0;

void Editor::CreateEditor()
{
	EventManager::OnMainLoop.push_back(OnMainLoop);
	EventManager::OnClick.push_back(OnClick);
	EventManager::OnCharPressed.push_back(OnCaracterInput);

	OpenEditor();
}

void Editor::OpenEditor()
{
	enabled = true;
	EventManager::Call(&EventManager::OnOpenEditor);
}

void Editor::CloseEditor()
{
	enabled = false;
	EventManager::Call(&EventManager::OnCloseEditor);
}

void Editor::DestroyEditor()
{
	CloseEditor();
}

void Editor::OnMainLoop()
{
	if (!enabled)
		return;

	DrawPanel();

	// Handle backspace for text input
	if (focusedTextInputID != "")
	{
		if (glfwGetKey(Utility::window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		{
			// Control: remove word
			if (glfwGetKey(Utility::window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(Utility::window, GLFW_KEY_RIGHT_CONTROL) )
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

void Editor::DrawPanel()
{
	// Start position: to left
	vec3 drawPos = vec3(10, Utility::screenY - textSize - margin, UIBaseZPos + 5);

	// Background
	(new Sprite(vec3(0, 0, UIBaseZPos), vec3(panelSize, Utility::screenY, UIBaseZPos), vec4(0, 0, 0, 0.3f)))->Draw();

	// Draw tabs buttons
	int i = 0;
	for (auto name = panelWindows.begin(); name != panelWindows.end(); name++)
	{
		bool pressed;
		drawPos.x += Button(drawPos, *name, &pressed).x + margin;

		if (pressed)
			currentPanelWindow = i;

		i++;
	}

	drawPos.x = 10;
	drawPos.y -= textSize + margin;

	switch (currentPanelWindow)
	{
	case 0:
		DrawPropsTab(drawPos);
		break;
	case 1:
		DrawAddTab(drawPos);
		break;
	case 2:
		DrawMapTab(drawPos);
		break;
	default:
		throw "Selected panel tab id is too high!";
		break;
	}
}

void Editor::DrawPropsTab(vec3 drawPos)
{
	if (GetSelectedObject() == NULL)
	{
		TextManager::RenderText("No selected object", drawPos, textSize);
	}
	else
	{
		drawPos.y -= TextManager::RenderText("Object properties", drawPos, textSize).y + margin;
		drawPos.x += indentation;
		drawPos.y -= GetSelectedObject()->DrawProperties(drawPos).y;
	}
}

void Editor::DrawAddTab(vec3 drawPos)
{
	drawPos.y -= TextManager::RenderText("Add an object", drawPos, textSize).y + margin;
	drawPos.x += indentation;

	bool pressed = false;
	void* newObject = nullptr;

	vec3 newPos = vec3(Camera::position.x, Camera::position.y, 0);

	drawPos.y -= Button(drawPos, "Sprite", &pressed).y;
	if (pressed)
		newObject = new EditorSprite(newPos);

	drawPos.y -= Button(drawPos, "Player", &pressed).y;
	if (pressed)
		newObject = new Player(newPos);

	if (newObject != nullptr)
	{
		editorObjects.push_back((EditorObject*)newObject);
		selectedObject = (EditorObject*)newObject;
	}
}

void Editor::DrawMapTab(vec3 drawPos)
{
	drawPos.y -= TextManager::RenderText("Map properties", drawPos, textSize).y + margin;
	drawPos.x += indentation;
}

void Editor::OnClick(GLFWwindow* window, int button, int action, int mods)
{
	if (!enabled)
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
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

bool Editor::isOverUI(vec2 point)
{
	return point.x > panelSize;
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
	catch (std::exception& _) { *value = 0.f; }

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
	catch (std::exception& _) { value->x = 0.f; }

	try { value->y = std::stof(resY); }
	catch (std::exception& _) { value->y = 0.f; }

	try { value->z = std::stof(resZ); }
	catch (std::exception& _) { value->z = 0.f; }

	try { value->w = std::stof(resW); }
	catch (std::exception& _) { value->w = 0.f; }

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
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
