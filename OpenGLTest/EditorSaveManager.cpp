#include "EditorSaveManager.h"

#include <string>

#include "Player.h"
#include "EditorSprite.h"
#include "Laser.h"
#include "Button.h"
#include "Light.h"
#include "ShadowCaster.h"
#include "LightManager.h"
#include "Prefab.h"
#include "Trigger.h"
#include "CameraController.h"
#include "LevelEnd.h"
#include "EditorParticleSystem.h"
#include "TweenManager.h"
#include "MenuManager.h"
#include "LogicRelay.h"
#include "TransformModifier.h"
#include "PrefabRelay.h"
#include "Acid.h"
#include "Checkpoint.h"
#include "TextRenderer.h"
#include "PhysicSimulation.h"
#include "PrefabCloner.h"
#include "Radioactivity.h"
#include "SoundPoint.h"
#include "SoundArea.h"
#include "TerminalWriter.h"
#include "TerminalManager.h"
#include "ObjectFollower.h"

using namespace glm;

std::list<EditorObject*> EditorSaveManager::levelObjectList = std::list<EditorObject*>();
MapData EditorSaveManager::currentMapData;

std::string EditorSaveManager::filePath = "";

const std::string EditorSaveManager::mapsBasePath = "Levels\\";
const std::string EditorSaveManager::settingsBasePath = "Settings\\";
std::ofstream* EditorSaveManager::ofile = nullptr;
std::ifstream* EditorSaveManager::ifile = nullptr;

const std::string EditorSaveManager::indentationString = "\t";
int EditorSaveManager::indentation = 0;

std::string EditorSaveManager::currentUserSave = "";

std::list<std::string> EditorSaveManager::userSaves = std::list<std::string>();

bool EditorSaveManager::isLoading = false;

void EditorSaveManager::ClearEditorLevel()
{
	std::cout << "Clearing editor level"  << std::endl;

	isLoading = true;

	// Destroy all objects
	Editor::SelectObject(nullptr);
	for (auto r = Editor::editorObjects.begin(); r != Editor::editorObjects.end(); r++)
	{
		delete *r;
	}
	Editor::editorObjects.clear();

	Editor::ClearUndoStack();
	Editor::ClearRedoStack();

	Editor::currentMapData = MapData();
	Editor::IDmax = 0;

	isLoading = false;
}

void EditorSaveManager::ClearGameLevel()
{
	std::cout << "Clearing game level" << std::endl;

	isLoading = true;

	for (auto r = levelObjectList.begin(); r != levelObjectList.end(); r++)
	{
		delete *r;
	}
	levelObjectList.clear();

	isLoading = false;
}

void EditorSaveManager::SaveLevel()
{
	if (!Editor::enabled)
	{
		std::cout << "You tried to save a level while the editor is disabled" << std::endl;
		return;
	}

	// No file path
	if (Editor::currentFilePath == "")
	{
		Editor::infoBarText = "Can't save the level, no save file path specified!!";
		std::cout << "Can't save the level, no save file path specified!!" << std::endl;
		return;
	}

	std::cout << "Saving level: " << Editor::currentFilePath << std::endl;

	// Open file
	ofile = new std::ofstream();
	ofile->open(mapsBasePath + Editor::currentFilePath, std::ios::out);

	// Can't open
	if (!ofile->is_open())
	{
		Editor::infoBarText = "Couldn't open level file for saving!";
		std::cout << "Couldn't open level file for saving!" << std::endl;
		return;
	}

	try
	{
		// Start to write file
		WriteHeader();
		WriteObject(Editor::currentMapData);

		for (auto r = Editor::editorObjects.begin(); r != Editor::editorObjects.end(); r++)
		{
			StartObject((*r)->typeName);
			(*r)->Save();
			EndObject();
		}
	}
	catch (std::exception e) // Catch exceptions to make sure the file is closed
	{
		ofile->close();
		throw e;
	}

	ofile->close();
}

void EditorSaveManager::LoadLevel(std::string path, bool inEditor)
{
	isLoading = true;

	if (inEditor)
	{
		Editor::currentFilePath = path;
		ClearEditorLevel();
		Camera::position = vec2(0, 0); // Reset camera position
	}
	else
	{
		ClearGameLevel();
		filePath = path;
	}

	TerminalManager::ClearTerminal();

	std::cout << "Loading level " << path << std::endl;

	ifile = new std::ifstream();
	ifile->open(mapsBasePath + path, std::ios::in);
	
	// Can't open
	if (!ifile->is_open())
	{
		Editor::infoBarText = "Oops! We couldn't open the level file!";
		std::cout << "Oops! We couldn't open the level file!" << std::endl;
		return;
	}

	try
	{
		// Start to read file
		FirstLineStartWith("count");
		int objectCount = std::stoi(ReadWord());

		for (int i = 0; i < objectCount; i++)
		{
			ReadObject(inEditor);
		}

		Editor::ClearUndoStack();
		Editor::ClearRedoStack();

		if (inEditor)
			Editor::infoBarText = "Loaded " + path;

		ifile->close();

		// Load prefabs inside level
		std::list<EditorObject*>* levelList = inEditor? &Editor::editorObjects : &levelObjectList;
		for (auto it = levelList->begin(); it != levelList->end(); it++)
		{
			if ((*it)->typeName == "Prefab" || (*it)->typeName == "PrefabCloner")
			{
				Prefab* prefab = (Prefab*)(*it);
				prefab->ReloadPrefab();
			}
		}

		LightManager::ForceRefreshLightmaps();

		if (!inEditor)
			Camera::SetSize(Camera::defaultSize);
	}
	catch (std::exception e) // Catch exceptions to make sure the file is closed
	{
		ifile->close();
		isLoading = false;
		Editor::infoBarText = "Got an error while loading level file, see console for details";
		std::cout << "Got an error while loading level file, here's what it says:" << std::endl;
		std::cout << e.what() << std::endl;

		// Make sure the editor stays empty
		if (inEditor)
		{
			ClearEditorLevel();
		}
		else
		{
			ClearGameLevel();
		}
	}

	isLoading = false;
}

void EditorSaveManager::LoadLevelWithTransition(std::string path, std::function<void()> onLoad)
{
	overlayZ = 2000;

	TweenManager<float>::Tween(0, 1, 2, [](float value) {
		overlayColor = vec4(0, 0, 0, value);
		globalVolumeOverride = 1 - value;
	}, linear)
	->SetOnFinished([path, onLoad] {
		// Load next level
		EditorSaveManager::LoadLevel(path, false);

		EventManager::DoInOneFrame([onLoad] {
			if (onLoad != nullptr)
				onLoad();

			// Fade out
			TweenManager<float>::Tween(1, 0, 2, [](float value) {
				overlayColor = vec4(0, 0, 0, value);
				globalVolumeOverride = 1 - value;
				}, linear);

			// Set camera instantly
			if (Camera::getTarget != nullptr)
				Camera::position = Camera::getTarget();
		});
	});
}

void EditorSaveManager::LoadPrefab(Prefab* prefab)
{
	if (prefab->GetPath() == "") return;

	//isLoading = true;

	std::cout << "Loading prefab " << prefab->GetPath() << std::endl;

	prefab->prefabRelay = nullptr;

	std::string path = mapsBasePath + prefab->GetPath();
	ifile->open(path, std::ios::in);

	// Can't open
	if (!ifile->is_open())
	{
		std::cout << "Oops! We couldn't open the prefab file!" << std::endl;
		return;
	}

	try
	{
		// Start to read file
		FirstLineStartWith("count");
		int objectCount = std::stoi(ReadWord());

		for (int i = 0; i < objectCount; i++)
		{
			// Load object
			ReadObject(false, prefab);

			if (prefab->prefabObjects.size() == 0) continue; // Map data loaded, ignore

			// Set parent to prefab if none
			EditorObject* newObj = prefab->prefabObjects.back();
			if (newObj->parentID == -1)
			{
				newObj->SetParent(prefab);
			}

			newObj->contextList = &prefab->prefabObjects;

			if (newObj->typeName == "PrefabRelay")
			{
				if (prefab->prefabRelay != nullptr)
					std::cout << "Two prefabRelay in one prefab!" << std::endl;
				else
					prefab->prefabRelay = (PrefabRelay*)newObj;
			}
		}

		ifile->close();

		// Load prefabs inside prefab
		for (auto it = prefab->prefabObjects.begin(); it != prefab->prefabObjects.end(); it++)
		{
			if ((*it)->typeName == "Prefab")
			{
				Prefab* prefab = (Prefab*)(*it);
				prefab->ReloadPrefab();
			}
		}

		//isLoading = false;
	}
	catch (std::exception e) // Catch exceptions to make sure the file is closed
	{
		ifile->close();
		//isLoading = false;
		std::cout << "Got an error while loading prefab file, here's what it says:" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

void EditorSaveManager::DisableEditorObjects()
{
	for (auto r = Editor::editorObjects.begin(); r != Editor::editorObjects.end(); r++)
	{
		(*r)->Disable();
	}
}

void EditorSaveManager::EnableEditorObjects()
{
	for (auto r = Editor::editorObjects.begin(); r != Editor::editorObjects.end(); r++)
	{
		(*r)->Enable();
	}
}

void EditorSaveManager::WriteHeader()
{
	(*ofile) << "// Editor map file, version 1\n\n";
	(*ofile) << "count " << std::to_string(Editor::editorObjects.size() + 1) << "\n\n";
}

void EditorSaveManager::WriteObject(MapData obj)
{
	StartObject("MapData");
	WriteProp("mapName", obj.mapName);
	WriteProp("lightmapStart", obj.lightmapStart);
	WriteProp("lightmapEnd", obj.lightmapEnd);
	EndObject();
}

void EditorSaveManager::WriteProp(std::string name, std::string value)
{
	WriteIndentation();
	(*ofile) << name << ": \"" << value << "\"\n";
}

void EditorSaveManager::WriteProp(std::string name, float value)
{
	EditorSaveManager::WriteProp(name, std::to_string(value));
}

void EditorSaveManager::WriteProp(std::string name, int value)
{
	EditorSaveManager::WriteProp(name, std::to_string(value));
}

void EditorSaveManager::WriteProp(std::string name, bool value)
{
	EditorSaveManager::WriteProp(name, std::string(value ? "1" : "0"));
}

void EditorSaveManager::WriteProp(std::string name, vec2 value)
{
	WriteIndentation();
	(*ofile) << name << ": \"" << std::to_string(value.x) << ", " << std::to_string(value.y) << "\"\n";
}

void EditorSaveManager::WriteProp(std::string name, vec3 value)
{
	WriteIndentation();
	(*ofile) << name << ": \"" << std::to_string(value.x) << ", " << std::to_string(value.y) << ", " << std::to_string(value.z) << "\"\n";
}

void EditorSaveManager::WriteProp(std::string name, vec4 value)
{
	WriteIndentation();
	(*ofile) << name << ": \"" << std::to_string(value.x) << ", " << std::to_string(value.y) << ", " << std::to_string(value.z) << ", " << std::to_string(value.w) << "\"\n";
}

void EditorSaveManager::WriteIndentation()
{
	for (int i = 0; i < indentation; i++)
	{
		(*ofile) << indentationString;
	}
}

void EditorSaveManager::StartObject(std::string type)
{
	WriteIndentation();
	(*ofile) << "object " << type << " {\n";
	indentation++;
}

void EditorSaveManager::EndObject()
{
	indentation--;
	WriteIndentation();
	(*ofile) << "}\n";
}

void EditorSaveManager::FirstLineStartWith(std::string s, bool mustThrow)
{
	std::string line = "";
	std::streampos lineStartPos;

	while (!ifile->eof())
	{
		lineStartPos = ifile->tellg();
		std::getline(*ifile, line);
		if (line.length() > 0)
			line = line.substr(line.find_first_not_of(" \n\r\t\f\v")); // Remove indentation and other whitespace characters

		bool passed = true;
		for (int i = 0; i < line.length() && i < s.length(); i++)
		{
			if (line[i] != s[i])
			{
				passed = false;
				break;
			}
		}

		if (passed)
		{
			ifile->seekg(lineStartPos);
			ifile->seekg(s.length() + 2, std::ios::cur);
			return;
		}
	}

	if (mustThrow)
		throw "Tried to find line starting with '" + s + "', but no line match!\n If it's not a problem change mustThrow parameter when calling the function";
}

std::string EditorSaveManager::ReadWord()
{
	std::string res = "";
	char ch = ' ';
	while (std::isspace(ch) && !ifile->eof())
	{
		ifile->get(ch);

		if (ifile->eof())
			throw "EOF found while reading word!";
	}

	while ((std::isalpha(ch) || std::isdigit(ch)) && !ifile->eof())
	{
		res += ch;
		ifile->get(ch);

		if (ifile->eof())
			throw "EOF found while reading word!";
	}

	return res;
}

std::string EditorSaveManager::ReadProp()
{
	std::string res = "";
	char ch = ' ';
	while (std::isspace(ch))
	{
		ifile->get(ch);
	}

	if (ch != '\"')
		throw "\" character was expected";

	ifile->get(ch);

	while (ch != '\"')
	{
		if (ifile->eof())
			throw "EOF found while reading property, some double quotes are missing";

		res += ch;
		ifile->get(ch);
	}

	return res;
}

void EditorSaveManager::GoToEndOfLine()
{
	char ch = ' ';
	while (ch != '\n' && !ifile->eof())
	{
		ifile->get(ch);
	}
}

void EditorSaveManager::ReadObject(bool inEditor, Prefab* prefab)
{
	FirstLineStartWith("object");
	std::string objectType = ReadWord(); // Read type name
	GoToEndOfLine();

	//// READ PROPERTIES
	std::map<std::string, std::string> props;

	std::string propName = " ";
	
	while (!ifile->eof())
	{
		propName = ReadWord();
		if (propName == "")
			break;
		ifile->ignore(1); // Ignore colons
		std::string propVal = ReadProp();
		props.insert(make_pair(propName, propVal));
		GoToEndOfLine();
	}

	if (ifile->eof())
		throw "EOF found while reading object, a '}' might me missing";

	//// Create object
	EditorObject* newObj = nullptr;

	if (objectType == "MapData")
	{
		MapData newData = MapData();
		newData.mapName = props["mapName"];
		newData.lightmapStart = StringToVector2(props["lightmapStart"]);
		newData.lightmapEnd = StringToVector2(props["lightmapEnd"]);

		if (prefab != nullptr)
			prefab->mapData = newData;
		else if (inEditor)
			Editor::currentMapData = newData;
		else
			currentMapData = newData;
	}
	else if (objectType == "Player")
	{
		newObj = (EditorObject*)(new Player(vec3(0)));
		newObj->Load(&props);
	}
	else if (objectType == "EditorSprite")
	{
		newObj = new EditorSprite();
		newObj->Load(&props);
	}
	else if (objectType == "Laser")
	{
		newObj = new Laser();
		newObj->Load(&props);
	}
	else if (objectType == "Button")
	{
		newObj = new Button(vec3(0));
		newObj->Load(&props);
	}
	else if (objectType == "Light")
	{
		newObj = new Light();
		newObj->Load(&props);
	}
	else if (objectType == "ShadowCaster")
	{
		newObj = new ShadowCaster();
		newObj->Load(&props);
	}
	else if (objectType == "Prefab")
	{
		newObj = new Prefab();
		newObj->Load(&props);
	}
	else if (objectType == "Trigger")
	{
		newObj = new Trigger();
		newObj->Load(&props);
	}
	else if (objectType == "CameraController")
	{
		newObj = new CameraController();
		newObj->Load(&props);
	}
	else if (objectType == "LevelEnd")
	{
		newObj = new LevelEnd();
		newObj->Load(&props);
	}
	else if (objectType == "EditorParticleSystem")
	{
		newObj = new EditorParticleSystem();
		newObj->Load(&props);
	}
	else if (objectType == "LogicRelay")
	{
		newObj = new LogicRelay();
		newObj->Load(&props);
	}
	else if (objectType == "TransformModifier")
	{
		newObj = new TransformModifier();
		newObj->Load(&props);
	}
	else if (objectType == "PrefabRelay")
	{
		newObj = new PrefabRelay();
		newObj->Load(&props);
	}
	else if (objectType == "Acid")
	{
		newObj = new Acid();
		newObj->Load(&props);
	}
	else if (objectType == "Checkpoint")
	{
		newObj = new Checkpoint();
		newObj->Load(&props);
	}
	else if (objectType == "TextRenderer")
	{
		newObj = new TextRenderer();
		newObj->Load(&props);
	}
	else if (objectType == "PhysicSimulation")
	{
		newObj = new PhysicSimulation();
		newObj->Load(&props);
	}
	else if (objectType == "PrefabCloner")
	{
		newObj = new PrefabCloner();
		newObj->Load(&props);
	}
	else if (objectType == "Radioactivity")
	{
		newObj = new Radioactivity();
		newObj->Load(&props);
	}
	else if (objectType == "SoundPoint")
	{
		newObj = new SoundPoint();
		newObj->Load(&props);
	}
	else if (objectType == "SoundArea")
	{
		newObj = new SoundArea();
		newObj->Load(&props);
	}
	else if (objectType == "TerminalWriter")
	{
		newObj = new TerminalWriter();
		newObj->Load(&props);
	}
	else if (objectType == "ObjectFollower")
	{
		newObj = new ObjectFollower();
		newObj->Load(&props);
	}
	else
		throw "Unknown object type for loading!";

	if (newObj != nullptr)
	{
		if (prefab != nullptr)
		{
			prefab->prefabObjects.push_back(newObj);
		}
		else if (inEditor) 
		{
			Editor::AddObject(newObj);

			if (newObj->ID > Editor::IDmax)
			{ 
				Editor::IDmax = newObj->ID + 1;
			}
		}
		else
		{
			levelObjectList.push_back(newObj);
		}
	}
}

vec2 EditorSaveManager::StringToVector2(std::string s, vec2 def)
{
	if (s.length() < 1) return def;

	size_t pos = s.find(',');
	std::string x = s.substr(0, pos);
	std::string y = s.substr(pos + 1, s.length() - pos - 1);
	return vec2(std::stof(x), std::stof(y));
}

vec3 EditorSaveManager::StringToVector3(std::string s, vec3 def)
{
	if (s.length() < 1) return def;

	size_t pos = s.find(',');
	std::string x = s.substr(0, pos);
	std::string yz = s.substr(pos + 1, s.length() - pos - 1);
	pos = yz.find(',');
	std::string y = yz.substr(0, pos);
	std::string z = yz.substr(pos + 1, s.length() - pos - 1);

	return vec3(std::stof(x), std::stof(y), std::stof(z));
}

vec4 EditorSaveManager::StringToVector4(std::string s, vec4 def)
{
	if (s.length() < 1) return def;

	size_t pos = s.find(',');
	std::string x = s.substr(0, pos);
	std::string yzw = s.substr(pos + 1, s.length() - pos - 1);
	pos = yzw.find(',');
	std::string y = yzw.substr(0, pos);
	std::string zw = yzw.substr(pos + 1, s.length() - pos - 1);
	pos = zw.find(',');
	std::string z = zw.substr(0, pos);
	std::string w = zw.substr(pos + 1, s.length() - pos - 1);

	return vec4(std::stof(x), std::stof(y), std::stof(z), std::stof(w));
}

void EditorSaveManager::FloatProp(std::map<std::string, std::string>* props, std::string name, float* value)
{
	std::string text = (*props)[name];

	if (text != "")
		*value = std::stof(text);
}

void EditorSaveManager::IntProp(std::map<std::string, std::string>* props, std::string name, int* value)
{
	std::string text = (*props)[name];

	if (text != "")
		*value = std::stoi(text);
}

void EditorSaveManager::ReadPropsFile(std::string fileName, std::map<std::string, std::string>* res)
{
	std::string path = fileName;

	ifile = new std::ifstream();
	ifile->open(path, std::ios::in);

	if (!ifile->is_open())
	{
		std::cout << "Couldn't open props file (" << path << ")" << std::endl;
		return;
	}

	try
	{
		FirstLineStartWith("Props");
		GoToEndOfLine();

		while (!ifile->eof())
		{
			std::string settingName = ReadWord();
			if (settingName == "")
				break;
			ifile->ignore(1); // Ignore colons
			std::string propVal = ReadProp();
			res->insert(make_pair(settingName, propVal));
			GoToEndOfLine();
		}

		std::cout << "Read props at " << path << std::endl;
		ifile->close();
	}
	catch (std::exception e) // Catch exceptions to make sure the file is closed
	{
		std::cout << "Got an error while loading props file (" << path << "), here's what it says:" << std::endl;
		std::cout << e.what() << std::endl;
		ifile->close();
	}
}

void EditorSaveManager::WritePropsFile(std::string fileName, std::function<void()> writer)
{
	std::string path = fileName;

	ofile = new std::ofstream();
	ofile->open(path, std::ios::out);

	if (!ofile->is_open())
	{
		std::cout << "Couldn't create props file (" << path << ")" << std::endl;
		return;
	}

	try
	{
		(*ofile) << "//Props file\n\n";
		(*ofile) << "Props {\n";
		
		indentation = 1;

		writer();

		indentation = 0;

		(*ofile) << "}\n";
		
		std::cout << "Written props at " << path << std::endl;
		ofile->close();
	}
	catch (std::exception e) // Catch exceptions to make sure the file is closed
	{
		std::cout << "Got an error while writing props file (" << path << "), here's what it says:" << std::endl;
		std::cout << e.what() << std::endl;
		ofile->close();
	}
}

void EditorSaveManager::LoadUserSave(std::string fileName)
{
	std::map<std::string, std::string> props = std::map<std::string, std::string>();
	ReadPropsFile("Saves\\" + fileName, &props);

	currentUserSave = fileName;

	LoadLevelWithTransition(props["level"], [fileName] {
		LoadUserSaveInSameLevel(fileName);
		MenuManager::OpenMenu(MenuManager::Menu::ingame);
	});
}

void EditorSaveManager::LoadUserSaveInSameLevel(std::string fileName)
{
	std::map<std::string, std::string> props = std::map<std::string, std::string>();
	ReadPropsFile("Saves\\" + fileName, &props);

	currentUserSave = fileName;

	vec2 position = StringToVector2(props["position"]);
	float camSize;
	FloatProp(&props, "cameraSize", &camSize);

	if (Player::ingameInstance != nullptr)
	{
		Player::ingameInstance->SetPos(position);
	}

	Camera::size = camSize;

	for (auto it = levelObjectList.begin(); it != levelObjectList.end(); it++)
	{
		(*it)->ResetIngameState();
	}
}

void EditorSaveManager::SaveUserSave(std::string fileName)
{
	WritePropsFile("Saves\\" + fileName, [] {
		WriteProp("level", filePath);
		WriteProp("position", Player::ingameInstance->GetPos());
		WriteProp("cameraSize", Camera::size);
	});
}

void EditorSaveManager::IndexUserSaves()
{
	userSaves.clear();

	std::string savesDir = "Saves";
	for (const auto& entry : std::filesystem::recursive_directory_iterator(savesDir))
	{
		std::string fileName = ToLower(entry.path().string());
		fileName = fileName.substr(savesDir.length() + 1, fileName.length() - savesDir.length() - 1); // Remove "saves\"
		userSaves.push_back(fileName);
	}
}

void EditorSaveManager::OnMainLoop()
{
	for (auto it = levelObjectList.begin(); it != levelObjectList.end(); it++)
	{
		(*it)->OnMainLoop();
	}
}
