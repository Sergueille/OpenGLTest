#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include "Editor.h"
#include "EditorObject.h"

using namespace glm;

struct MapData;
class EditorObject;
class EditorSaveManager
{
public:
	static std::list<EditorObject*> levelObjectList;
	static MapData currentMapData;

	/// <summary>
	/// The level folder path
	/// </summary>
	static const std::string mapsBasePath;

	/// <summary>
	/// String used to indentation (ex: two spaces, a tabulation)
	/// </summary>
	static const std::string indentationString;

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

	/// <summary>
	/// Makes all object of the editor disabled
	/// </summary>
	static void DisableEditorObjects();
	/// <summary>
	/// Makes all object of the editor enabled
	/// </summary>
	static void EnableEditorObjects();

	static void WriteProp(std::string name, std::string value);
	static void WriteProp(std::string name, bool value);
	static void WriteProp(std::string name, vec2 value);
	static void WriteProp(std::string name, vec3 value);
	static void WriteProp(std::string name, vec4 value);
	static void WriteIndentation();
	static void StartObject(std::string type);
	static void EndObject();

	static void FloatProp(std::map<std::string, std::string>* props, std::string name, float* value);

	static vec2 StringToVector2(std::string s, vec2 def = vec2(0));
	static vec3 StringToVector3(std::string s, vec3 def = vec3(0));
	static vec4 StringToVector4(std::string s, vec4 def = vec4(0));
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

	static void ReadObject(bool inEditor);
};
