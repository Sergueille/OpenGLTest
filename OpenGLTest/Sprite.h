#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <queue>

#include "Texture.h"
#include "mesh.h"
#include "shader.h"

// POSSIBLE MEMORY LEAK!!! delete static mesh 

struct CompareSprite;
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
	bool isTransparent;

	/// <summary>
	/// Should the sprite be auto-destroyed when drawn
	/// </summary>
	bool destroyAfterDrawing;

	static std::priority_queue<Sprite*, std::vector<Sprite*>, CompareSprite> drawQueue;

	Sprite(Texture* texture, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), 
		float rotate = 0.0f, glm::vec4 color = glm::vec4(1.0f), Shader* shader = nullptr, 
		bool isUI = false, bool isTransparent = true, bool destroyAfterDrawing = false);

	Sprite(bool isUI, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), 
		glm::vec4 color = glm::vec4(1.0f), bool isTransparent = true, bool destroyAfterDrawing = false);
	Sprite(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(0, 0, 0, 1));

	// DEBUG DESTRUCTOR
	~Sprite();

	/// <summary>
	/// Draw now if opaque, or add sprite to queue if transparent
	/// </summary>
	void Draw();

	/// <summary>
	/// Draw all sprites stored in queue
	/// </summary>
	static void DrawAll();

private:
	void DrawNow();
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

struct CompareSprite
{
	bool operator()(const Sprite& lhs, const Sprite& rhs);
};
