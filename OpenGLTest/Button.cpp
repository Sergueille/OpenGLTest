#include "Button.h"

#include "CircleCollider.h"
#include "Player.h"

Button::Button(vec3 position) : EditorObject(position)
{
	isPressed = false;

	btnSprite = new Sprite(
		RessourceManager::GetTexture("switch_on.png"),
		position, vec2(1.16f),
		0, vec4(1.3, 1.3, 1.3, 1)
	);
	btnSprite->isLit = true;
	btnSprite->DrawOnMainLoop();

	clickCollider = new CircleCollider(position, .5f, false);

	typeName = "Button";

	if (!Editor::enabled)
	{
		// Subscribe to events
		keyFuncPos = EventManager::OnKeyPressed.push_end(
			[this] (GLFWwindow* window, int key, int scancode, int action, int mods) 
			{ this->OnKeyPressed(key, action); }
		);
	}
}

Button::~Button()
{
	delete btnSprite;
	btnSprite = nullptr;

	if (!Editor::enabled)
	{
		EventManager::OnKeyPressed.remove(keyFuncPos);
	}
}

EditorObject* Button::Copy()
{
	Button* newObj = new Button(*this);

	CircleCollider* oldCollider = (CircleCollider*)clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

	newObj->btnSprite = this->btnSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

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
	
	drawPos.y -= Editor::CheckBox(drawPos, "Start pressed", &startPressed, Editor::panelPropertiesX).y;

	drawPos.y -= onPressed.DrawInPanel(drawPos, "On pressed").y;
	drawPos.y -= onUnpressed.DrawInPanel(drawPos, "On unpressed").y;

	vec2 sizeRes = vec2(drawPos) - startPos;
	sizeRes *= -1;
	return sizeRes;
}

bool Button::GetState()
{
	return isPressed;
}

void Button::SetState(bool value, bool sendEvents)
{
	isPressed = value;

	if (isPressed)
	{
		if (sendEvents)
			onPressed.Call(this);

		btnSprite->texture = RessourceManager::GetTexture("switch_on.png");
	}
	else
	{
		if (sendEvents)
			onUnpressed.Call(this);

		btnSprite->texture = RessourceManager::GetTexture("switch_off.png");
	}
}

void Button::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("startPressed", startPressed);
	EditorSaveManager::WriteProp("onPressed", onPressed.GetString());
	EditorSaveManager::WriteProp("onUnpressed", onUnpressed.GetString());
}

void Button::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	EventList::Load(&onPressed, (*props)["onPressed"]);
	EventList::Load(&onUnpressed, (*props)["onUnpressed"]);

	startPressed = (*props)["startPressed"] == "1";

	SetState(startPressed, false);
}

void Button::OnKeyPressed(int key, int action)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		if (!interactAlreadyPressed)
		{
			vec2 playerPos = vec2(Player::ingameInstance->GetPos());
			vec2 myPos = vec2(GetEditPos());
			float dist = glm::length(playerPos - myPos);

			if (dist < interactDistance)
			{
				SetState(!GetState());
			}

			interactAlreadyPressed = true;
		}
	}
	else
	{
		interactAlreadyPressed = false;
	}
}
