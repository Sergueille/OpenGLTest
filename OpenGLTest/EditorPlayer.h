#pragma once

#include "EditorObject.h"

class EditorPlayer : EditorObject
{
public:
	EditorPlayer(vec2 pos);
	~EditorPlayer();

	Sprite* sprite = NULL;

	virtual vec2 SetPos(vec2 pos) override;
};

