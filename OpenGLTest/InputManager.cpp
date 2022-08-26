#include "InputManager.h"

#include "SettingsManager.h"
#include "Utility.h"
#include "LocalizationManager.h"

int InputManager::GetPrimKey(KeyBinding key)
{
	int id = (int)key;
	std::string setting = "key" + std::to_string(id);
	return SettingsManager::GetIntSetting(setting);
}

int InputManager::GetAltKey(KeyBinding key)
{
	int id = (int)key;
	std::string setting = "keyalt" + std::to_string(id);
	return SettingsManager::GetIntSetting(setting);
}

bool InputManager::IsPressed(KeyBinding action)
{
	return glfwGetKey(Utility::window, GetPrimKey(action)) == GLFW_PRESS
		|| glfwGetKey(Utility::window, GetAltKey(action)) == GLFW_PRESS;
}

void InputManager::SetKey(KeyBinding key, int value, bool isPrim)
{
	int id = (int)key;

	std::string setting;
	if (isPrim)
		setting = "key" + std::to_string(id);
	else
		setting = "keyalt" + std::to_string(id);

	SettingsManager::SetIntSetting(setting, value);
	SettingsManager::SaveSettings();
}

std::string InputManager::GetKeyName(KeyBinding action, bool prim)
{
	int key;
	if (prim)
		key = GetPrimKey(action);
	else
		key = GetAltKey(action);

	std::string localKey;

	if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
		return std::string(glfwGetKeyName(key, 0));
	else if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
	{
		char c = std::toupper(glfwGetKeyName(key, 0)[0]);
		std::string s = std::string();
		s.push_back(c);
		return s;
	}
	else if (key == GLFW_KEY_ENTER)
		localKey = "enter";
	else if (key == GLFW_KEY_SPACE)
		localKey = "space";
	else if (key == GLFW_KEY_TAB)
		localKey = "tab";
	else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
		localKey = "shift";
	else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
		localKey = "control";
	else if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)
		localKey = "alt";
	else if (key == GLFW_KEY_COMMA)
		localKey = "comma";
	else if (key == GLFW_KEY_LEFT)
		localKey = "left_arrow";
	else if (key == GLFW_KEY_RIGHT)
		localKey = "right_arrow";
	else if (key == GLFW_KEY_UP)
		localKey = "up_arrow";
	else if (key == GLFW_KEY_DOWN)
		localKey = "down_arrow";
	else
		localKey = "unknown_key";

	return LocalizationManager::GetLocale(localKey);
}
