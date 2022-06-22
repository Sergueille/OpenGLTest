#include "Light.h"

#include "RessourceManager.h"
#include "CircleCollider.h"
#include "Utility.h"
#include "LightManager.h"

Light::Light() : EditorObject(vec3(0))
{
	if (Editor::enabled)
	{
		editorSprite = new Sprite(RessourceManager::GetTexture("engine\\pointLight.png"), vec3(0));
		editorSprite->DrawOnMainLoop();
		LightManager::lights.push_back(this);
	}

	clickCollider = new CircleCollider(vec3(0), 1, false);

	typeName = "Light";
}

Light::~Light()
{
	if (editorSprite != nullptr) // In editor
	{
		delete editorSprite;
		editorSprite = nullptr;

		LightManager::lights.remove(this);
	}
}

void Light::OnMainLoop()
{
	EditorObject::OnMainLoop();

	if (enabled && editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}

void Light::Enable()
{
	EditorObject::Enable();
	if (editorSprite != nullptr)
	{
		editorSprite->DrawOnMainLoop();
		LightManager::lights.push_back(this);
	}
}

void Light::Disable()
{
	EditorObject::Disable();
	if (editorSprite != nullptr)
	{
		editorSprite->StopDrawing();
		LightManager::lights.remove(this);
	}
}

vec2 Light::DrawProperties(vec3 drawPos)
{
	vec2 startPos = vec2(drawPos);

	std::string strID = std::to_string(ID);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Size", &size, Editor::panelPropertiesX, strID + "size").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Color", &color, Editor::panelPropertiesX, strID + "color", true).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Intensity", &intensity, Editor::panelPropertiesX, strID + "intensity").y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Rotation", &editorRotation, Editor::panelPropertiesX, strID + "angle").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Inner angle", &innerAngle, Editor::panelPropertiesX, strID + "inner").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Outer angle", &outerAngle, Editor::panelPropertiesX, strID + "outer").y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Shadow size", &shadowSize, Editor::panelPropertiesX, strID + "shadowSize").y;

	return Abs(startPos - vec2(drawPos));
}

void Light::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("lightSize", size);
	EditorSaveManager::WriteProp("color", color);
	EditorSaveManager::WriteProp("intensity", intensity);

	EditorSaveManager::WriteProp("rotation", editorRotation);
	EditorSaveManager::WriteProp("innerAngle", innerAngle);
	EditorSaveManager::WriteProp("outerAngle", outerAngle);

	EditorSaveManager::WriteProp("shadowSize", shadowSize);
}

void Light::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props); 

	EditorSaveManager::FloatProp(props, "lightSize", &size);
	color = EditorSaveManager::StringToVector4((*props)["color"]);
	EditorSaveManager::FloatProp(props, "intensity", &intensity);

	EditorSaveManager::FloatProp(props, "rotation", &editorRotation);
	EditorSaveManager::FloatProp(props, "innerAngle", &innerAngle);
	EditorSaveManager::FloatProp(props, "outerAngle", &outerAngle);

	EditorSaveManager::FloatProp(props, "shadowSize", &shadowSize);
}

EditorObject* Light::Copy()
{
	Light* copy = new Light(*this);

	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	copy->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, false);

	if (editorSprite != nullptr)
	{
		copy->editorSprite = this->editorSprite->Copy();
		LightManager::lights.push_back(copy);
	}

	return copy;
}
