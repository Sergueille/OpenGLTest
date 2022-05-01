#include "LightManager.h"

#include "RessourceManager.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image.h"

std::list<Light*> LightManager::lights = std::list<Light*>();
int LightManager::pixelsPerUnit = 5;

unsigned int LightManager::texID = 0;
std::string LightManager::texLevelPath = "";

void LightManager::BakeLight()
{
	Shader* shader = &RessourceManager::shaders["lightmapper"];

	constexpr int MAX_LIGHT_COUNT = 256;

	if (lights.size() > MAX_LIGHT_COUNT)
	{
		std::cout << "Max light count exceeded, please changhe the array sizes in the lightmapper shader" << std::endl;
	}

	// Get light data
	vec2 levelMin; vec2 levelMax;
	Editor::GetLevelAABB(&levelMin, &levelMax);
	vec2 levelSize = levelMax - levelMin;

	GLfloat posArray[MAX_LIGHT_COUNT * 2] = {};
	GLfloat colorArray[MAX_LIGHT_COUNT * 3] = {};
	GLfloat sizeArray[MAX_LIGHT_COUNT * 2] = {};

	int i = 0;
	for (auto it = lights.begin(); it != lights.end(); it++, i++)
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
	glBindTexture(GL_TEXTURE_2D, colorTex); // Bind colorTex
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind colorTex
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0); // Bind color texture to buffer

	// Setup shader
	shader->Use();
	shader->SetUniform("nbLights", (int)lights.size());
	shader->SetUniform2f("lightPos", posArray, MAX_LIGHT_COUNT);
	shader->SetUniform3f("lightColor", colorArray, MAX_LIGHT_COUNT);
	shader->SetUniform2f("lightSize", sizeArray, MAX_LIGHT_COUNT);

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

	ForceRefreshLightmaps();
}

unsigned int LightManager::GetLightData()
{
	if (Editor::currentFilePath == texLevelPath)
		return texID;

	texLevelPath = Editor::currentFilePath;

	std::string path = "Lightmaps\\" + Editor::currentFilePath + ".hdr";

	// Read texture file
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (nrChannels != 3)
	{
		std::cout << "Ooops! The lightmap image hae wrong amount of channels" << std::endl;
		return 0;
	}

	if (data) 
	{
		// Create texture
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		std::cout << "Loaded texture " << path << std::endl;
	}
	else 
	{
		std::cout << "Failed to read level lighmap at " << path << std::endl;
		return 0;
	}
}

void LightManager::ForceRefreshLightmaps()
{
	texLevelPath = "";
}
