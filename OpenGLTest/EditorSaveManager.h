#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include "Editor.h"
#include "EditorObject.h"

using namespace glm;

struct MapData;
class EditorSaveManager
{
public:
	/// <summary>
	/// The level folder path
	/// </summary>
	static const std::string mapsBasePath;

	static const std::string indentationString;

	static void ClearLevel();

	static void SaveLevel();
	static void LoadLevel(std::string path);

	static void WriteProp(std::string name, std::string value);
	static void WriteProp(std::string name, vec2 value);
	static void WriteProp(std::string name, vec3 value);
	static void WriteProp(std::string name, vec4 value);
	static void WriteIndentation();
	static void StartObject(std::string type);
	static void EndObject();

	static vec2 StringToVector2(std::string s);
	static vec3 StringToVector3(std::string s);
	static vec4 StringToVector4(std::string s);
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

	static void ReadObject();
};
