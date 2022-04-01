#include "EditorPlayer.h"

#include "RessourceManager.h"
#include "Player.h"
#include "CircleCollider.h"

using namespace glm;

EditorPlayer::EditorPlayer(vec2 pos) : EditorObject(pos)
{
	this->sprite = new Sprite(&RessourceManager::textures["circle"], vec3(pos.x, pos.y, -2), vec2(1), 0.f, vec4(1, 0, 0, 1));
	this->clickCollider = new CircleCollider(pos, 1);
}

EditorPlayer::~EditorPlayer()
{
	delete sprite;
}

vec2 EditorPlayer::SetPos(vec2 pos)
{
	sprite->position = vec3(pos.x, pos.y, 3);
	return EditorObject::SetPos(pos);
}
