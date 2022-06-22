#include "ShadowCaster.h"

#include "RessourceManager.h"
#include "CircleCollider.h"
#include "LightManager.h"

ShadowCaster::ShadowCaster() : EditorObject(vec3(0))
{
	if (Editor::enabled)
	{
		editorSprite = new Sprite(
			RessourceManager::GetTexture("Engine\\shadowCaster.png"),
			vec3(0), vec2(1), 0
		);
		editorSprite->DrawOnMainLoop();

		areaSprite = new Sprite(nullptr, vec3(0), vec2(1), 0, vec4(0, 1, 0, 0.5f));

		LightManager::shadowCasters.push_back(this);
	}

	clickCollider = new CircleCollider(vec3(0), 1, false);

	typeName = "ShadowCaster";
}

ShadowCaster::~ShadowCaster()
{
	if (editorSprite != nullptr) // If in editor
	{
		delete editorSprite;
		editorSprite = nullptr;

		delete areaSprite;
		areaSprite = nullptr;

		if (enabled)
			LightManager::shadowCasters.remove(this);
	}
}

void ShadowCaster::OnMainLoop()
{
	EditorObject::OnMainLoop();

	if (!enabled || !Editor::enabled) return;

	editorSprite->position = GetEditPos();

	areaSprite->position = GetEditPos() + vec3(0, 0, -0.1f);
	areaSprite->rotate = GetEditRotation();
	areaSprite->size = GetEditScale();

	editorSprite->size = vec2(Editor::gizmoSize);
	((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
}

vec2 ShadowCaster::DrawProperties(vec3 drawPos)
{
	vec2 startPos = vec2(drawPos);

	std::string strID = std::to_string(ID);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Size", &editorSize, Editor::panelPropertiesX, strID + "size").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &editorRotation, Editor::panelPropertiesX, strID + "orientation").y;

	return Abs(startPos - vec2(drawPos));
}

void ShadowCaster::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);
	EditorSaveManager::FloatProp(props, "orientation", &editorRotation);
	editorSize = EditorSaveManager::StringToVector2((*props)["size"]);
}

void ShadowCaster::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("size", editorSize);
	EditorSaveManager::WriteProp("orientation", editorRotation);
}

void ShadowCaster::OnSelected()
{
	areaSprite->DrawOnMainLoop();
}

void ShadowCaster::OnUnselected()
{
	areaSprite->StopDrawing();
}

EditorObject* ShadowCaster::Copy()
{
	ShadowCaster* copy = new ShadowCaster(*this);
	
	copy->clickCollider = new CircleCollider(vec2(0), 1, false);

	if (editorSprite != nullptr)
	{
		copy->editorSprite = this->editorSprite->Copy();
		copy->areaSprite = this->areaSprite->Copy();
		LightManager::shadowCasters.push_back(copy);
	}

	return copy;
}

void ShadowCaster::Enable()
{
	EditorObject::Enable();
	editorSprite->DrawOnMainLoop();

	if (Editor::enabled)
	{
		LightManager::shadowCasters.push_back(this);
	}
}

void ShadowCaster::Disable()
{
	EditorObject::Disable();
	editorSprite->StopDrawing();
	areaSprite->StopDrawing();

	if (Editor::enabled)
	{
		LightManager::shadowCasters.remove(this);
	}
}
