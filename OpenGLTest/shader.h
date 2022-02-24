#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include "shader.h"

class Shader
{
public:
	/// <summary>
	/// Shader program ID, use this to reference it with OpenGL
	/// </summary>
	unsigned int ID;
	/// <summary>
	/// Create a shader by reading files
	/// </summary>
	Shader(const char* vertexPath, const char* fragmentPath);
	/// <summary>
	/// Activates the shader, nessesary to render or change uniforms
	/// </summary>
	void Use();

	// Set uniforms
	void SetUniform(const char* uniformName, bool value);
	void SetUniform(const char* uniformName, float value);
	void SetUniform(const char* uniformName, int value);
	void SetUniform(const char* uniformName, float x, float y, float z, float w);

private:
	/// <summary>
	/// Checks if the shader could be compiled and display an error message
	/// </summary>
	/// <param name="shaderID">The ID of the shader object</param>
	static void CheckShaderCompilation(unsigned int shaderID, const char* path);
};

#endif