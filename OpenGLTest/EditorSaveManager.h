#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include "Editor.h"
#include "EditorObject.h"

using namespace glm;

struct MapData;
class EditorObject;
class Prefab;
class EditorSaveManager
{
public:
	static std::list<EditorObject*> levelObjectList;
	static MapData currentMapData;

	/// <summary>
	/// The path of the level that is loaded in game
	/// </summary>
	static std::string filePath;

	/// <summary>
	/// The level folder path
	/// </summary>
	static const std::string mapsBasePath;
	/// <summary>
	/// The settings folder path
	/// </summary>
	static const std::string settingsBasePath;

	/// <summary>
	/// String used to indentation (ex: two spaces, a tabulation)
	/// </summary>
	static const std::string indentationString;

	/// <summary>
	/// User save that is currnetly being used to save the game
	/// </summary>
	static std::string currentUserSave;

	/// <summary>
	/// List of all the user saves, created by IndexUserSaves()
	/// </summary>
	static std::list<std::string> userSaves;

	/// <summary>
	/// Destroy all editor objects
	/// </summary>
	static void ClearEditorLevel();

	/// <summary>
	/// Destroy all objects of the game level
	/// </summary>
	static void ClearGameLevel();

	/// <summary>
	/// Save all editor object into a file
	/// </summary>
	static void SaveLevel();
	static void LoadLevel(std::string path, bool inEditor = false);

	static void LoadLevelWithTransition(std::string path, std::function<void()> onLoad = nullptr);

	static void LoadPrefab(Prefab* prefab);

	/// <summary>
	/// Makes all object of the editor disabled
	/// </summary>
	static void DisableEditorObjects();
	/// <summary>
	/// Makes all object of the editor enabled
	/// </summary>
	static void EnableEditorObjects();

	static void WriteProp(std::string name, std::string value);
	static void WriteProp(std::string name, float value);
	static void WriteProp(std::string name, int value);
	static void WriteProp(std::string name, bool value);
	static void WriteProp(std::string name, vec2 value);
	static void WriteProp(std::string name, vec3 value);
	static void WriteProp(std::string name, vec4 value);
	static void WriteIndentation();
	static void StartObject(std::string type);
	static void EndObject();

	static void FloatProp(std::map<std::string, std::string>* props, std::string name, float* value);
	static void IntProp(std::map<std::string, std::string>* props, std::string name, int* value);

	static vec2 StringToVector2(std::string s, vec2 def = vec2(0));
	static vec3 StringToVector3(std::string s, vec3 def = vec3(0));
	static vec4 StringToVector4(std::string s, vec4 def = vec4(0));

	static void ReadPropsFile(std::string fileName, std::map<std::string, std::string>* res);
	static void WritePropsFile(std::string fileName, std::function<void()> writer);

	static void LoadUserSave(std::string fileName);
	static void LoadUserSaveInSameLevel(std::string fileName);
	static void SaveUserSave(std::string fileName);

	static void IndexUserSaves();

	static void OnMainLoop();

private:
	static std::ofstream* ofile;
	static std::ifstream* ifile;
	static int indentation;

	static void WriteHeader();
	static void WriteObject(MapData obj);

	/// <summary>
	/// Go to first line starting with s, after s
	/// </summary>
	static void FirstLineStartWith(std::string s, bool mustThrow = true);
	/// <summary>
	/// Return all characters until a non-alpha and non-digit char is found
	/// </summary>
	static std::string ReadWord();
	static std::string ReadProp();
	static void GoToEndOfLine();

	static void ReadObject(bool inEditor, Prefab* prefab = nullptr);
};
