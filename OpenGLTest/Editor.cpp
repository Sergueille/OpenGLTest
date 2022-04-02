#include "Editor.h"

#include "CircleCollider.h"
#include "Utility.h"

#include <iostream>

using namespace glm;

std::list<EditorObject*> Editor::editorObjects;
EditorObject* Editor::selectedObject;

const float Editor::selectClickMargin = 0.1f;

const vec4 Editor::textColor = vec4(1);
const vec4 Editor::highlightColor = vec4(1, 0.5, 0.8, 1);
const vec4 Editor::editColor = vec4(1, 0.2, 0.5, 1);;

std::string Editor::focusedTextInputID = "";
std::string Editor::focusedTextInputValue = "";
float Editor::backspaceNextTime = 0;

void Editor::SetupEditor()
{
	EventManager::OnMainLoop.push_back(OnMainLoop);
	EventManager::OnClick.push_back(OnClick);
	EventManager::OnCharPressed.push_back(OnCaracterInput);
}

void Editor::OnMainLoop()
{
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
	vec2 drawPos = vec2(10, Utility::screenY - textSize - margin);

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

void Editor::OnClick(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		vec2 worldPos = Utility::ScreenToWorld(Utility::GetMousePos());
		CircleCollider mouseColl = CircleCollider(worldPos, selectClickMargin);

		for (auto obj = editorObjects.begin(); obj != editorObjects.end(); obj++)
		{
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
	if (focusedTextInputID != "")
	{
		focusedTextInputValue += char(codepoint);
	}
}

vec2 Editor::TextInput(vec2 pos, std::string* value, std::string ID, TextManager::text_align align)
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

vec2 Editor::DrawProperty(vec2 drawPos, const std::string name, std::string* value, float propX, std::string ID)
{
	TextManager::RenderText(name + ":", drawPos, textSize);
	drawPos.x += propX;
	vec2 propSize = TextInput(drawPos, value, ID);

	return vec2(propX + propSize.x, textSize);
}

vec2 Editor::DrawProperty(vec2 drawPos, const std::string name, float* value, float propX, std::string ID)
{
	std::string res = std::to_string(*value);
	vec2 size = DrawProperty(drawPos, name, &res, propX, ID);

	try { *value = std::stof(res); }
	catch (std::exception& e) { *value = 0.f; }

	return size;
}

vec2 Editor::DrawProperty(vec2 drawPos, const std::string name, vec2* value, float propX, std::string ID)
{
	std::string resX = std::to_string(value->x);
	std::string resY = std::to_string(value->y);

	drawPos.y -= TextManager::RenderText(name + ":", drawPos, textSize).y;
	TextManager::RenderText("X:", drawPos + vec2(indentation, 0), textSize);
	vec2 Xsize = TextInput(drawPos + vec2(propX, 0), &resX, ID + "X");
	drawPos.y -= Xsize.y;

	TextManager::RenderText("Y:", drawPos + vec2(indentation, 0), textSize);
	vec2 Ysize = TextInput(drawPos + vec2(propX, 0), &resY, ID + "Y");

	float maxX = max(Xsize.x, Ysize.x);

	try { value->x = std::stof(resX); }
	catch (std::exception& e) { value->x = 0.f; }

	try { value->y = std::stof(resY); }
	catch (std::exception& e) { value->y = 0.f; }

	return vec2(propX + maxX, textSize * 3);
}
