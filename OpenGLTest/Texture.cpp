#include <iostream>
#include "stb_image.h"
#include <glad/glad.h> 

#include "Texture.h"

Texture::Texture(const char* path)
{
	// Read texture file
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	if (data) 
	{
		// Create texture
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
	{
		std::cout << "You tried to loat the texture " << path << ", but stb_image didn't want to do it." << std::endl;
		ID = NULL;
	}

	// Clear memory
	stbi_image_free(data);
}