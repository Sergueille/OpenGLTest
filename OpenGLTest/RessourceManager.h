#pragma once

#include <map>
#include <string>
#include <glad/glad.h>
#include <map>

#include "texture.h"
#include "shader.h"

class RessourceManager
{
public:
	static std::map<std::string, Shader> shaders;
	static std::map<std::string, Texture> textures;

	static Shader* LoadShader(const char* vtex, const char* frag, std::string programName);
	static Texture* LoadTexture(const char* file, std::string textureName);

	static void Clear();
};

