#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"
#include "EditorSprite.h"

using namespace glm;

constexpr int RADIOACTIVITY_EVENT_COUNT = 2;

class Radioactivity : public EditorObject
{
public:
	Radioactivity();
	~Radioactivity();

    Sprite* editorSprite = nullptr;

	float maxDist = 0;
	bool isRealTime = false;

	int regionID = -1;

	static ObjectEvent events[RADIOACTIVITY_EVENT_COUNT];

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

	virtual void ResetIngameState() override;

	Mesh* GetMesh();
	void ForceRefreshMesh();

private:
	Mesh* mesh = nullptr;

	LinkedListElement<std::function<void()>>* mainLoopFuncPos = nullptr;

	bool startedRealtime = false;

	void CreateVertex(float* vertices, int* vertexCount, vec2 pos);
	void CreateTriangle(unsigned int* indices, int* vertexCount, int* faceCount, int raycastCount);

	void OnAfterMainLoop();
};

struct RaycastPoint {
	vec2 delta;
	float angle;
	int coll_id;
};
