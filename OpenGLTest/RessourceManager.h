#pragma once

#include <map>
#include <string>
#include <glad/glad.h>
#include <map>
#include <algorithm>
#include <soloud_wav.h>

#include "texture.h"
#include "shader.h"

class RessourceManager
{
public:
	static std::map<std::string, Shader> shaders;
	static std::map<std::string, Texture*> textures;
	static std::map<std::string, SoLoud::Wav*> sounds;

	static Shader* LoadShader(const char* vtex, const char* frag, std::string programName);

	static Texture* GetTexture(std::string file);
	static SoLoud::Wav* GetSound(std::string file);

	static void Clear();
};

