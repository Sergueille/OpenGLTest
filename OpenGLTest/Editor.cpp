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

void Editor::SetupEditor()
{
	EventManager::OnMainLoop.push_back(OnMainLoop);
	EventManager::OnClick.push_back(OnClick);
}

void Editor::OnMainLoop()
{
	DrawPanel();
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

std::tuple<vec2, std::string> Editor::TextInput(vec2 pos, std::string value, std::string ID)
{
	if (focusedTextInputID == ID)
	{
		vec2 textRect = TextManager::RenderText(focusedTextInputValue, pos, textSize, TextManager::right, editColor);
		return { textRect, value };
	}
	else
	{
		vec2 mousePos = Utility::GetMousePos();
		mousePos.y *= -1;
		mousePos.y += Utility::screenY;
		vec2 textRect = TextManager::GetRect(value, textSize);

		bool hoverX = mousePos.x > pos.x && mousePos.x < pos.x + textRect.x;
		bool hoverY = mousePos.y > pos.y && mousePos.y < pos.y + textRect.y;
		
		vec4 color;
		if (hoverX && hoverY)
		{
			if (glfwGetMouseButton(Utility::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				focusedTextInputID = ID;
				focusedTextInputValue = value;
			}

			float lerpValue = (float)cos(Utility::time * Utility::PI / (hoverHighlightDuration / 1000.f)) * 0.5 + 0.5;
			color = highlightColor + (textColor - highlightColor) * lerpValue;
		}
		else
		{
			color = textColor;
		}

		TextManager::RenderText(value, pos, textSize, TextManager::right, color);
		return { textRect, value };
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

vec2 Editor::DrawProperty(vec2 drawPos, std::string name, std::string value, float sizeX)
{
	TextManager::RenderText(name, drawPos, textSize);
	drawPos.x += sizeX;
	TextManager::RenderText(value, drawPos, textSize, TextManager::left);

	return vec2(sizeX, textSize);
}

vec2 Editor::DrawProperty(vec2 drawPos, std::string name, vec2 value, float sizeX)
{
	drawPos.y -= TextManager::RenderText(name, drawPos, textSize).y;
	drawPos.x += indentation;
	TextManager::RenderText("X:", drawPos, textSize);
	drawPos.y -= TextManager::RenderText(
		std::to_string(value.x), 
		drawPos + vec2(sizeX - indentation, 0),
		textSize, TextManager::left).y;

	TextManager::RenderText("Y:", drawPos, textSize);
	drawPos.y -= TextManager::RenderText(
		std::to_string(value.y), 
		drawPos + vec2(sizeX - indentation, 0),
		textSize, TextManager::left).y;

	return vec2(sizeX + indentation, textSize * 3);
}
