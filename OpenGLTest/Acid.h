#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class Acid : public EditorObject
{
public:
	Acid();
	~Acid();

    Sprite* sprite = nullptr;

	bool showSurface = true;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

private:
	static void SetSpriteUniforms(Shader* shader, void* object);

	static bool isPlayerDying;

	LinkedListElement<std::function<void()>>* acidMainLoopFuncPos = nullptr;
	void OnAcidMainLoop();
};

