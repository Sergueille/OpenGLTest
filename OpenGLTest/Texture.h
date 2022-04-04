#pragma once

#include <string>

class Texture
{
public:
	/// <summary>
	/// The ID of the texture
	/// </summary>
	unsigned int ID;
	/// <summary>
	/// Native ratio of the image, used by spriteRenderer to scale automatically
	/// ( width / height )
	/// </summary>
	float ratio;

	/// <summary>
	/// The path of the original image file, NOT including base path and "Images\"
	/// </summary>
	std::string path;

	/// <summary>
	/// Could this texture be loaded
	/// </summary>
	bool loadingFailed;

	/// <summary>
	/// Creates a new texture form file
	/// Enter path from executable position, including "Images"
	/// </summary>
	Texture(std::string path);
	/// <summary>
	/// DO NOT USE ME! I'm the default constructor!
	/// </summary>
	Texture();
	/// <summary>
	/// Use this to put texture in a shader, just before rendering
	/// </summary>
	/// <param name="unit">The position of the texture in shader params (0-15)</param>
	void Use(int unit);
};
