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

	Sprite(Shader* shader, Texture* texture, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), 
		float rotate = 0.0f, glm::vec4 color = glm::vec4(1.0f));
	void Draw();
};

namespace SpriteRenderer
{
	static Mesh* mesh;
	static Mesh* GetMesh();
};