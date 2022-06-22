#include "LightManager.h"

#include "RessourceManager.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image.h"

std::list<Light*> LightManager::lights = std::list<Light*>();
std::list<ShadowCaster*> LightManager::shadowCasters = std::list<ShadowCaster*>();
int LightManager::pixelsPerUnit = 20; // Must be multiple of 2 !!

unsigned int LightManager::texID = 0;
std::string LightManager::texLevelPath = "";

bool LightManager::mustReadNewFile = false;

void LightManager::BakeLight()
{
	Shader* shader = &RessourceManager::shaders["lightmapper"];

	constexpr int MAX_LIGHT_COUNT = 256;
	constexpr int MAX_SHADOW_CASTERS_COUNT = 128;

	if (lights.size() > MAX_LIGHT_COUNT)
	{
		std::cout << "Max light count exceeded, please changhe the array sizes in the lightmapper shader" << std::endl;
	}
	if (shadowCasters.size() > MAX_SHADOW_CASTERS_COUNT)
	{
		std::cout << "Max shodow casters count exceeded, please changhe the array sizes in the lightmapper shader" << std::endl;
	}

	// Get light data
	vec2 levelMin; vec2 levelMax;
	GetLevelAABB(&levelMin, &levelMax, Editor::enabled);

	// Round to make sure the total amount of pixels is multiple of 4
	levelMin = RoundVector(levelMin);
	levelMax = RoundVector(levelMax);

	vec2 levelSize = levelMax - levelMin;

	float posArray[MAX_LIGHT_COUNT * 2] = {};
	float colorArray[MAX_LIGHT_COUNT * 3] = {};
	float sizeArray[MAX_LIGHT_COUNT * 2] = {};
	float angleArray[MAX_LIGHT_COUNT * 3] = {};
	float shadowSizeArray[MAX_LIGHT_COUNT * 2] = {};

	int i = 0;
	for (auto it = lights.begin(); it != lights.end(); it++, i++)
	{
		if ((*it)->IsEnabled())
		{
			vec2 uvPos = (vec2((*it)->GetEditPos()) - levelMin);
			uvPos.x /= levelSize.x;
			uvPos.y /= levelSize.y;

			posArray[2 * i] = uvPos.x;
			posArray[2 * i + 1] = uvPos.y;

			colorArray[3 * i] = (*it)->color.r * (*it)->intensity;
			colorArray[3 * i + 1] = (*it)->color.g * (*it)->intensity;
			colorArray[3 * i + 2] = (*it)->color.b * (*it)->intensity;

			sizeArray[2 * i] = (*it)->size / levelSize.x;
			sizeArray[2 * i + 1] = (*it)->size / levelSize.y;

			angleArray[3 * i] = (*it)->GetEditRotation();
			angleArray[3 * i + 1] = (*it)->innerAngle;
			angleArray[3 * i + 2] = (*it)->outerAngle;

			shadowSizeArray[2 * i] = (*it)->shadowSize / levelSize.x;
			shadowSizeArray[2 * i + 1] = (*it)->shadowSize / levelSize.y;
		}
	}

	float shadowPos[MAX_SHADOW_CASTERS_COUNT * 2] = {};
	float shadowRot[MAX_SHADOW_CASTERS_COUNT] = {};
	float shadowScale[MAX_SHADOW_CASTERS_COUNT * 2] = {};

	i = 0;
	for (auto it = shadowCasters.begin(); it != shadowCasters.end(); it++, i++)
	{
		if ((*it)->IsEnabled())
		{
			vec2 uvPos = (vec2((*it)->GetEditPos()) - levelMin);
			uvPos.x /= levelSize.x;
			uvPos.y /= levelSize.y;

			shadowPos[2 * i] = uvPos.x;
			shadowPos[2 * i + 1] = uvPos.y;

			shadowScale[2 * i] = (*it)->GetEditScale().x / levelSize.x;
			shadowScale[2 * i + 1] = (*it)->GetEditScale().y / levelSize.y;

			shadowRot[i] = (*it)->GetEditRotation() + 0.1f; // Because don't likes vertival lines
		}
	}

	ivec2 resolution = ivec2(
		static_cast<int>(levelSize.x * (float)pixelsPerUnit),
		static_cast<int>(levelSize.y * (float)pixelsPerUnit)
	);

	// Create a frame buffer and texture
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); // Bind fbo
	unsigned int colorTex;
	glGenTextures(1, &colorTex);
	BindTexture2D(colorTex); // Bind colorTex
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0); // Bind color texture to buffer
	glViewport(0, 0, resolution.x, resolution.y);

	// Setup shader
	shader->Use();
	shader->SetUniform("nbLights", (int)lights.size());
	shader->SetUniform2f("lightPos", posArray, (int)lights.size());
	shader->SetUniform3f("lightColor", colorArray, (int)lights.size());
	shader->SetUniform2f("lightSize", sizeArray, (int)lights.size());
	shader->SetUniform3f("lightAngles", angleArray, (int)lights.size());
	shader->SetUniform2f("lightShadowSize", shadowSizeArray, (int)lights.size());

	shader->SetUniform("nbShadowCasters", (int)shadowCasters.size());
	shader->SetUniform2f("shadowCastersPos", shadowPos, (int)shadowCasters.size());
	shader->SetUniform("shadowCastersRot", shadowRot, (int)shadowCasters.size());
	shader->SetUniform2f("shadowCastersSize", shadowScale, (int)shadowCasters.size());

	vec2 quadSize = vec2(screenX, screenY);
	quadSize.x /= (float)resolution.x;
	quadSize.y /= (float)resolution.y;
	shader->SetUniform("quadSize", quadSize);

	// Reder
	SpriteRenderer::GetMesh()->DrawMesh(); // steal the quad of the sprite renderer

	// Read buffer data
	int nbPixels = resolution.x * resolution.y * 3;
	float* pixels = new float[nbPixels];

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, resolution.x, resolution.y, GL_RGB, GL_FLOAT, pixels);

	// Write file
	std::string path = "Lightmaps\\" + Editor::currentFilePath + ".hdr";
	stbi_flip_vertically_on_write(true);
	stbi_write_hdr(path.c_str(), resolution.x, resolution.y, 3, pixels);

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind fbo
	glViewport(0, 0, Utility::screenX, Utility::screenY);

	Editor::currentMapData.lightmapStart = levelMin;
	Editor::currentMapData.lightmapEnd = levelMax;

	delete[] pixels;

	ForceRefreshLightmaps();
}

unsigned int LightManager::GetLightData()
{
	if (!mustReadNewFile)
		return texID;

	glDeleteTextures(1, &texID);

	texLevelPath = Editor::enabled? Editor::currentFilePath : EditorSaveManager::filePath;
	mustReadNewFile = false;

	std::string path = "Lightmaps\\" + texLevelPath + ".hdr";

	// Read texture file
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);

	if (nrChannels != 3)
	{
		std::cout << "Ooops! The lightmap image has wrong amount of channels" << std::endl;
		stbi_image_free(data);
		return 0;
	}

	if (data) 
	{
		// Create texture
		glGenTextures(1, &texID);
		BindTexture2D(texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		std::cout << "Loaded texture " << path << std::endl;
		return texID;
	}
	else 
	{
		std::cout << "Failed to read level lighmap at " << path << std::endl;
		stbi_image_free(data);
		return 0;
	}

	stbi_image_free(data);
}

void LightManager::ForceRefreshLightmaps()
{
	mustReadNewFile = true;
}
