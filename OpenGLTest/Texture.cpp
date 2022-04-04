#include <iostream>
#include "stb_image.h"
#include <glad/glad.h> 

#include "Texture.h"

Texture::Texture(std::string path)
{
	this->path = path;
	this->path.erase(0, std::string("Images\\").length());

	// Read texture file
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data) 
	{
		// Create texture
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		int type = nrChannels == 3 ? GL_RGB : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		loadingFailed = false;
		std::cout << "Loaded texture " << path << std::endl;
	}
	else 
	{
		loadingFailed = true;
		std::cout << "Failed to load " << path << ", stb_image didn't want to do it." << std::endl;
		ID = NULL;
	}

	ratio = (float)width / (float)height;

	// Clear memory
	stbi_image_free(data);
}

Texture::Texture() { }

void Texture::Use(int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}
