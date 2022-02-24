#pragma once
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
	/// Creates a new texture form file
	/// </summary>
	Texture(const char* path);
	/// <summary>
	/// Use this to put texture in a shader, just before rendering
	/// </summary>
	/// <param name="unit">The position of the texture in shader params (0-15)</param>
	void Use(int unit);
};
