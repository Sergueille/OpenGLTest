#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

constexpr int PREFAB_MAX_EVENT_COUNT = 8;

using namespace glm;

class PrefabRelay;
class Prefab : public EditorObject
{
public:
	Prefab();
	virtual ~Prefab();

    Sprite* editorSprite = nullptr;

	std::list<EditorObject*> prefabObjects;
	PrefabRelay* prefabRelay;

	MapData mapData;

	std::string GetPath();
	void SetPath(std::string newName);

	virtual void ReloadPrefab();

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual vec2 DrawActions(vec3 drawPos) override;

	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetAABB(vec2* minRes, vec2* maxRes);

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	virtual void ResetIngameState() override;

private:
	std::string prefabPath = "";
	
	int eventCount = 0;
	ObjectEvent events[PREFAB_MAX_EVENT_COUNT];
};
