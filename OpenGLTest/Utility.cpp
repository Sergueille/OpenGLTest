#include "Utility.h"

namespace Utility
{
	GLFWwindow* window = NULL;
	float time = 0;
	float lastTime = 0;

	float GetDeltaTime()
	{
		return time - lastTime;
	}
}
