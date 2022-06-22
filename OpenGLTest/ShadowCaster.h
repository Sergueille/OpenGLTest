#pragma once
#include "EditorObject.h"
class ShadowCaster : public EditorObject
{
public:
	ShadowCaster();
	~ShadowCaster();

	Sprite* editorSprite = nullptr;
	Sprite* areaSprite = nullptr;

	virtual EditorObject* Copy() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void OnMainLoop() override;

	virtual vec2 DrawProperties(vec3 drawPos) override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void OnSelected() override;
	virtual void OnUnselected() override;
};

