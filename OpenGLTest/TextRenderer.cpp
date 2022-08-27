#include "TextRenderer.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "LocalizationManager.h"

TextRenderer::TextRenderer() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\textRenderer.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "TextRenderer";
}

TextRenderer::~TextRenderer()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }
}

vec2 TextRenderer::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Content", &content, Editor::panelPropertiesX, strID + "content").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Font size", &fontSize, Editor::panelPropertiesX, strID + "fontSize").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Color", &color, Editor::panelPropertiesX, strID + "color", true).y;

	drawPos.y -= Editor::CheckBox(drawPos, "Insert key names", &insertKeyNames, Editor::panelPropertiesX).y;

	if (insertKeyNames)
	{
		const char** keyBiningsNames = new const char* [5] {"Left", "Right", "Jump", "Down", "Use" };

		int value = (int)keyName1;
		drawPos.y -= Editor::OptionProp(
			drawPos, "First key", 
			&value, (int)InputManager::KeyBinding::keyCount - 1, 
			keyBiningsNames, Editor::panelPropertiesX).y;
		keyName1 = (InputManager::KeyBinding)value;

		value = (int)keyName2;
		drawPos.y -= Editor::OptionProp(
			drawPos, "Second key",
			&value, (int)InputManager::KeyBinding::keyCount - 1,
			keyBiningsNames, Editor::panelPropertiesX).y;
		keyName2 = (InputManager::KeyBinding)value;
	}

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* TextRenderer::Copy()
{
	TextRenderer* newObj = new TextRenderer(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	return newObj;
}

void TextRenderer::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	content = (*props)["content"];
	EditorSaveManager::FloatProp(props, "fontSize", &fontSize);
	color = EditorSaveManager::StringToVector4((*props)["color"], vec4(1));

	insertKeyNames = (*props)["insertKeyNames"] == "1";

	int keyint;
	EditorSaveManager::IntProp(props, "keyName1", &keyint);
	keyName1 = (InputManager::KeyBinding)keyint;
	EditorSaveManager::IntProp(props, "keyName2", &keyint);
	keyName2 = (InputManager::KeyBinding)keyint;
}

void TextRenderer::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("content", content);
	EditorSaveManager::WriteProp("fontSize", fontSize);
	EditorSaveManager::WriteProp("color", color);

	EditorSaveManager::WriteProp("insertKeyNames", insertKeyNames);
	EditorSaveManager::WriteProp("keyName1", (int)keyName1);
	EditorSaveManager::WriteProp("keyName2", (int)keyName2);
}

void TextRenderer::Enable()
{
	if (enabled) return;

	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void TextRenderer::Disable()
{
	if (!enabled) return;

	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void TextRenderer::OnMainLoop()
{
	EditorObject::OnMainLoop();

	if (!enabled) return;
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	float screenSize = fontSize / Camera::size * screenY;
	vec3 screenPos = Utility::WorldToScreen(GetEditPos());
	screenPos.z = GetEditPos().z;

	std::string txt = LocalizationManager::GetLocale(content);

	if (insertKeyNames)
	{
		txt = std::vformat(txt, std::make_format_args(InputManager::GetKeyName(keyName1, true), InputManager::GetKeyName(keyName2, true)));
	}

	TextManager::RenderText(txt, screenPos, screenSize, TextManager::center, color);
}
