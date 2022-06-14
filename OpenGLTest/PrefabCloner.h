#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "Prefab.h"

using namespace glm;

struct PrefabClone;
class PrefabCloner : public Prefab
{
public:
	PrefabCloner();
	~PrefabCloner();

	std::list<PrefabClone*> clonedObjects;

	bool cloneOnTimer = true;
	float delay = 2;

	bool deleteAfterTime = false;
	float deleteTime;

	int maxInstances = 10;

	virtual void ReloadPrefab() override;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	void CreateClone();

private:
	float lastTime = 0;
};

struct PrefabClone
{
	std::list<EditorObject*> objects;
	float startTime;

	~PrefabClone();
};
