#include <iostream>
#include "stb_image.h"
#include <glad/glad.h> 

#include "Texture.h"

Texture::Texture(const char* path)
{
	// Read texture file
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) 
	{
		// Create texture
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		int type = nrChannels == 3 ? GL_RGB : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
	{
		std::cout << "You tried to load the texture " << path << ", but stb_image didn't want to do it." << std::endl;
		ID = NULL;
	}

	ratio = (float)width / (float)height;

	// Clear memory
	stbi_image_free(data);
}

void Texture::Use(int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}
