#include "Button.h"

#include "CircleCollider.h"

Button::Button(vec3 position) : EditorObject(position)
{
	isPressed = false;

	btnSprite = new Sprite(
		RessourceManager::GetTexture("engine\\circle.png"),
		position, vec2(.5f),
		0, vec4(1, 0, 0, 1)
	);
	btnSprite->DrawOnMainLoop();

	clickCollider = new CircleCollider(position, .5f, false);

	typeName = "Button";
}

Button::~Button()
{
	delete btnSprite;
	btnSprite = nullptr;
}

EditorObject* Button::Copy()
{
	Button* newObj = new Button(*this);

	CircleCollider* oldCollider = (CircleCollider*)clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->position, oldCollider->size, oldCollider->MustCollideWithPhys());

	newObj->btnSprite = this->btnSprite->Copy();

	return newObj;
}

void Button::UpdateTransform()
{
	EditorObject::UpdateTransform();
	btnSprite->position = GetEditPos();
}

void Button::Enable()
{
	EditorObject::Enable();
	btnSprite->DrawOnMainLoop();
}

void Button::Disable()
{
	EditorObject::Disable();
	btnSprite->StopDrawing();
}

vec2 Button::DrawProperties(vec3 drawPos)
{
	vec2 startPos = vec2(drawPos);
	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	std::string strID = std::to_string(ID);
	
	drawPos.y -= onPressed.DrawInPanel(drawPos, "On pressed").y + Editor::margin;
	drawPos.y -= onUnpressed.DrawInPanel(drawPos, "On unpressed").y;

	vec2 sizeRes = vec2(drawPos) - startPos;
	sizeRes *= -1;
	return sizeRes;
}

bool Button::GetState()
{
	return isPressed;
}

void Button::SetState(bool value)
{
	isPressed = value;

	if (isPressed)
	{
		onPressed.Call();
		btnSprite->color = vec4(0, 1, 0, 1);
	}
	else
	{
		onUnpressed.Call();
		btnSprite->color = vec4(1, 0, 0, 1);
	}
}

void Button::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("onPressed", onPressed.GetString());
	EditorSaveManager::WriteProp("onUnpressed", onUnpressed.GetString());
}

void Button::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	EventList::Load(&onPressed, (*props)["onPressed"]);
	EventList::Load(&onUnpressed, (*props)["onUnpressed"]);
}
