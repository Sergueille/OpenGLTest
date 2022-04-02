#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"
#include "mesh.h"
#include "shader.h"

// POSSIBLE MEMORY LEAK!!! delete static mesh 

class Sprite 
{
public:
	Shader* shader;
	Texture* texture;
	glm::vec3 position;
	glm::vec2 size;
	float rotate;
	glm::vec4 color;
	bool isUI;

	Sprite(Texture* texture, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), 
		float rotate = 0.0f, glm::vec4 color = glm::vec4(1.0f), Shader* shader = nullptr, bool isUI = false);

	Sprite(bool isUI, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), glm::vec4 color = glm::vec4(1.0f));

	static void DrawSprite(Texture* texture, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1),
		float rotate = 0.0f, glm::vec4 color = glm::vec4(1.0f), Shader* shader = nullptr, bool isUI = false);

	static void DrawSpriteUI(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(0, 0, 0, 1));

private:
	void Draw();
};

namespace SpriteRenderer
{
	/// <summary>
	/// Mesh used to draw sprites
	/// </summary>
	static Mesh* mesh;
	/// <summary>
	/// Get sprite mesh
	/// </summary>
	static Mesh* GetMesh();
};