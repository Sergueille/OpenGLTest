#pragma once
#include "EditorObject.h"

#include "RessourceManager.h"

class Button : public EditorObject
{
public:
	Button(vec3 position);
	~Button();

	Sprite* btnSprite;

	EventList onPressed;
	EventList onUnpressed;

	virtual void UpdateTransform() override;
	virtual EditorObject* Copy() override;

	virtual void Enable();
	virtual void Disable();

	virtual vec2 DrawProperties(vec3 drawPos) override;

	bool GetState();
	void SetState(bool value);

	virtual void Save() override;
	virtual void Load(std::map < std::string, std::string>* props) override;

private:
	bool isPressed;
};
