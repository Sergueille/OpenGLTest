#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class Radioactivity : public EditorObject
{
public:
	Radioactivity();
	~Radioactivity();

    Sprite* editorSprite = nullptr;

	int nbRaycast = 72;
	bool isRealTime = false;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	Mesh* GetMesh();
	void ForceRefreshMesh();

private:
	Mesh* mesh = nullptr;

	void CreateVertex(float* vertices, int* vertexCount, vec2 pos);
	void CreateTriangle(unsigned int* indices, int* vertexCount, int* faceCount, int raycastCount);
};

struct RaycastPoint {
	vec2 delta;
	float angle;
	int coll_id;
};
