#pragma once

#include <string>

class InputManager
{
public:
	enum class KeyBinding {
		left,
		right,
		jump,
		down,
		use,
		keyCount
	};

	static int GetPrimKey(KeyBinding key);
	static int GetAltKey(KeyBinding key);
	static bool IsPressed(KeyBinding action);
	static void SetKey(KeyBinding key, int value, bool isPrim);
	static std::string GetKeyName(KeyBinding key, bool prim);
};

