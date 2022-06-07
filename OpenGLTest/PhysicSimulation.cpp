#include "PhysicSimulation.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

ObjectEvent PhysicSimulation::events[PHYS_SIM_EVENT_COUNT] = {
	ObjectEvent {
		"Start simulation",
		[](EditorObject* object, void* param) { ((PhysicSimulation*)object)->StartSimulation(); },
	}
};

PhysicSimulation::PhysicSimulation() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\simulation.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "PhysicSimulation";
}

PhysicSimulation::~PhysicSimulation()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	UnloadSim();
}

vec2 PhysicSimulation::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

    drawPos.y -= Editor::DrawProperty(drawPos, "Scale", &editorSize, Editor::panelPropertiesX, strID + "size").y;

	std::string oldName = filename;
    drawPos.y -= Editor::DrawProperty(drawPos, "Simulation file name", &filename, Editor::panelPropertiesX, strID + "filename").y;
	if (oldName != filename)
	{
		LoadFile();
	}

	bool pressed;
	drawPos.y -= Editor::UIButton(drawPos, "Start", &pressed).y;
	if (pressed)
	{
		StartSimulation();
	}

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* PhysicSimulation::Copy()
{
	PhysicSimulation* newObj = new PhysicSimulation(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->fileLoaded = false;
	newObj->objects = nullptr;
	newObj->isSimulating = false;

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void PhysicSimulation::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

    editorSize = EditorSaveManager::StringToVector2((*props)["scale"]);
	filename = (*props)["filename"];

	LoadFile();
}

void PhysicSimulation::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("scale", editorSize);
	EditorSaveManager::WriteProp("filename", filename);
}

void PhysicSimulation::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();

	if (fileLoaded)
	{
		for (int i = 0; i < objectCount; i++)
		{
			objects[i].sprite->DrawOnMainLoop();
		}
	}
}

void PhysicSimulation::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();

	if (fileLoaded)
	{
		for (int i = 0; i < objectCount; i++)
		{
			objects[i].sprite->StopDrawing();
		}
	}
}

void PhysicSimulation::LoadFile()
{
	UnloadSim();

	if (filename == "")
	{
		std::cerr << "No file specified for physics simulation!" << std::endl;
		return;
	}

	std::string path = SIM_FILES_FOLDER + filename;
	std::ifstream* file = new std::ifstream(path, std::ios::in | std::ios::binary);

	// Can't open
	if (!file->is_open())
	{
		std::cerr << "Oops! We couldn't open the physic simulation file at " << path << "!" << std::endl;
		delete file;
		return;
	}

	try
	{
		char res[4];

		////// HEAD
		file->read(res, 4);
		if (res[0] != 'h' || res[1] != 'e')
			throw std::exception("File head not found!");

		// Read base infos
		file->read((char*)(&objectCount), sizeof(int));
		file->read((char*)(&FPS), sizeof(int));
		file->read((char*)(&frameCount), sizeof(int));

		objects = new PhysicSimObject[objectCount]; // Create objects

		float deltaZ = 0; // Add this to z to avoid z-fighting

		for (int i = 0; i < objectCount; i++)
		{
			objects[i] = PhysicSimObject();
			file->read((char*)&objects[i].size, sizeof(float)); // Set sizes

			objects[i].X = new float[frameCount]; // init arrays
			objects[i].Y = new float[frameCount];
			objects[i].rotation = new float[frameCount];

			objects[i].sprite = new Sprite(nullptr, vec3(0), vec2(1), 0, vec4(0, 0, 0, 1)); // init sprites
			objects[i].sprite->isLit = false;
			objects[i].z = deltaZ;
			objects[i].sprite->DrawOnMainLoop();
			objects[i].SetSpriteSize(this);
			deltaZ += 0.01f;

			int meshID;
			file->read((char*)&meshID, sizeof(int)); // Get mesh ID

			objects[i].sprite->texture = RessourceManager::GetTexture(DEBRIS_TEX_NAMES[meshID]);
		}

		////// BODY
		file->read(res, 4);
		if (res[0] != 'b' || res[1] != 'o')
			throw std::exception("File body not found!");

		for (int frame = 0; frame < frameCount; frame++)
		{
			for (int i = 0; i < objectCount; i++) // Read movement data
			{
				file->read((char*)&objects[i].X[frame], sizeof(float));
				file->read((char*)&objects[i].Y[frame], sizeof(float));
				file->read((char*)&objects[i].rotation[frame], sizeof(float));

				if (frame == 0) // Set sprite start pos
				{
					objects[i].SetSpritePos(this, 0);
					objects[i].SetSpriteRot(this, 0);
				}
			}
		}

		fileLoaded = true;
		file->close();
		delete file;
		std::cout << "Loaded simulation file: " << path << std::endl;
	}
	catch (std::exception e) // Catch exceptions to make sure the file is closed
	{
		std::cerr << "Got an error while loading physic simulation file, here's what it says:" << std::endl;
		std::cerr << e.what() << std::endl;
		file->close();
		delete file;
	}
}

void PhysicSimulation::UnloadSim()
{
	if (!fileLoaded) return;
	fileLoaded = false;

	delete[] objects;
}

void PhysicSimulation::StartSimulation()
{
	if (!fileLoaded) return;

	simulationStartTime = Utility::time;
	isSimulating = true;
}

void PhysicSimulation::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = events;
	*resCount = PHYS_SIM_EVENT_COUNT;
}

void PhysicSimulation::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos() + vec3(0, 0, 50);
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	if (fileLoaded)
	{
		int currentFrame;

		if (isSimulating)
		{
			float deltaT = Utility::time - simulationStartTime;
			float currentFrameFloat = deltaT * 60;
			currentFrame = std::floor(currentFrameFloat);

			if (currentFrame >= frameCount)
			{
				isSimulating = false;
			}
		}
		else if (Editor::enabled)
		{
			currentFrame = 0;
		}

		if (isSimulating || Editor::enabled)
		{
			for (int i = 0; i < objectCount; i++)
			{
				objects[i].SetSpritePos(this, currentFrame);
				objects[i].SetSpriteRot(this, currentFrame);

				if (Editor::enabled)
					objects[i].SetSpriteSize(this);
			}
		}
	}
}

PhysicSimObject::~PhysicSimObject()
{
	delete[] X;
	delete[] Y;
	delete[] rotation;
	delete sprite;
}

void PhysicSimObject::SetSpriteSize(PhysicSimulation* parent)
{
	vec2 size = parent->GetEditScale() * this->size;
	sprite->size = size;
}

void PhysicSimObject::SetSpritePos(PhysicSimulation* parent, int frame)
{
	vec3 pos = parent->GetEditPos() + vec3(
		parent->GetEditScale().x * this->X[frame], 
		parent->GetEditScale().y * this->Y[frame], 
		parent->GetEditPos().z + this->z);
	sprite->position = pos;
}

void PhysicSimObject::SetSpriteRot(PhysicSimulation* parent, int frame)
{
	float ori = this->rotation[frame];
	sprite->rotate = ori * Utility::RadToDeg;
}
