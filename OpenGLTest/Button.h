#pragma once
#include "EditorObject.h"

#include "RessourceManager.h"

class Button : public EditorObject
{
public:
	Button(vec3 position);
	~Button();

	const float interactDistance = 2;

	bool startPressed = false;

	Sprite* btnSprite;

	EventList onPressed;
	EventList onUnpressed;

	virtual void UpdateTransform() override;
	virtual EditorObject* Copy() override;

	virtual void Enable();
	virtual void Disable();

	virtual vec2 DrawProperties(vec3 drawPos) override;

	bool GetState();
	void SetState(bool value, bool sendEvents = true);

	virtual void Save() override;
	virtual void Load(std::map < std::string, std::string>* props) override;

private:
	bool isPressed;

	bool interactAlreadyPressed = false;

	LinkedListElement<std::function<void(GLFWwindow* window, int key, int scancode, int action, int mods)>>* keyFuncPos;
	void OnKeyPressed(int key, int action);
};
