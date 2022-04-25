#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <queue>
#include <list>
#include <functional>

#include "LinkedList.h"
#include "Texture.h"
#include "mesh.h"
#include "shader.h"

// POSSIBLE MEMORY LEAK!!! delete static mesh 

using namespace Utility;

typedef void (SetSpriteShaderUniforms) (Shader* shader, void* object);

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
	glm::vec2 UVStart;
	glm::vec2 UVEnd;
	bool forceOpaque;

	SetSpriteShaderUniforms* setUniforms;
	void* setUniformsObjectCall = nullptr;

	static std::priority_queue<Sprite*, std::vector<Sprite*>, CompareSprite> drawQueue;

	Sprite(Texture* texture, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), 
		float rotate = 0.0f, glm::vec4 color = glm::vec4(1.0f), Shader* shader = nullptr, 
		bool isUI = false);

	Sprite(bool isUI, glm::vec3 position = glm::vec3(0), glm::vec2 size = glm::vec2(1), 
		glm::vec4 color = glm::vec4(1.0f));
	Sprite(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(0, 0, 0, 1));

	virtual ~Sprite();

	/// <summary>
	/// Draw now if opaque, or add sprite to queue if transparent
	/// </summary>
	void Draw();

	/// <summary>
	/// Make this sprite draw himself automatically on main loop
	/// </summary>
	void DrawOnMainLoop();
	/// <summary>
	/// Tell this sprite to stop drawing himself
	/// </summary>
	void StopDrawing();

	bool IsTransparent();

	/// <summary>
	/// Draw all sprites stored in queue, MUST BE CALLED ONCE PER FRAME
	/// </summary>
	static void DrawAll();

	Sprite* Copy();

protected:
	bool isDrawnOnMainLoop;

private:
	LinkedListElement<std::function<void()>>* autoDrawFunc = nullptr;

	/// <summary>
	/// Force the sprite to be drawn RIGHT NOW, even if he's transparent
	/// </summary>
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
	bool operator()(Sprite* lhs, Sprite* rhs);
};
