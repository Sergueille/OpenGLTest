#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

constexpr const char* SIM_FILES_FOLDER = "Simulations\\";
constexpr const char* DEBRIS_TEX_NAMES[6] = {
	"Engine\\notFound.png",
	"rock_1.png",
	"rock_2.png",
	"rock_3.png",
	"rock_4.png",
	"rock_5.png",
};

constexpr int PHYS_SIM_EVENT_COUNT = 1;

struct PhysicSimObject;
class PhysicSimulation : public EditorObject
{
public:
	PhysicSimulation();
	~PhysicSimulation();

    Sprite* editorSprite = nullptr;
	static ObjectEvent events[PHYS_SIM_EVENT_COUNT];

	std::string filename;

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void OnMainLoop() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;

	void LoadFile();
	void UnloadSim();

	void StartSimulation();

	virtual void ResetIngameState() override;

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount) override;

private:
	bool fileLoaded = false;

	int objectCount;
	int FPS;
	int frameCount;

	PhysicSimObject* objects = nullptr;

	bool isSimulating = false;
	float simulationStartTime;
};

struct PhysicSimObject 
{
	~PhysicSimObject();

	float size;
	float z;

	float* X;
	float* Y;
	float* rotation;

	Sprite* sprite;

	void SetSpriteSize(PhysicSimulation* parent);
	void SetSpritePos(PhysicSimulation* parent, int frame);
	void SetSpriteRot(PhysicSimulation* parent, int frame);
};

