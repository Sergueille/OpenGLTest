#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class Prefab : public EditorObject
{
public:
	Prefab();
	~Prefab();

    Sprite* editorSprite = nullptr;

	std::list<EditorObject*> prefabObjects;

	MapData mapData;

	std::string GetPath();
	void SetPath(std::string newName);

	void ReloadPrefab();

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual vec2 DrawActions(vec3 drawPos) override;

	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetAABB(vec2* minRes, vec2* maxRes);

private:
	std::string prefabPath = "";
};
