#pragma once

#include <string>
#include <glad/glad.h> 
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "Sprite.h"
#include "RessourceManager.h"
#include "EventManager.h"
#include "Utility.h"

class Player
{
public:
	const float height = 1;
	/// <summary>
	/// Walk speed of the player, in units/sec
	/// </summary>
	const float speed = 4;

	Sprite* sprite = NULL;
	glm::vec2 position;

	Player();
	~Player();

private:
	void OnMainLoop();
};

