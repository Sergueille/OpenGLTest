#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include "Utility.h"
#include <list>
#include <functional>

namespace EventManager
{
	extern std::list<std::function<void()>> OnMainLoop;
	extern std::list<std::function<void()>> OnExitApp;
	extern std::list<GLFWmousebuttonfun> OnClick;
	extern std::list<GLFWscrollfun> OnScroll;

	void SetupEvents();

	/// <summary>
	/// Call an event
	/// </summary>
	void Call(std::list<std::function<void()>>* eventList);

	void OnClickCallback(GLFWwindow* window, int button, int action, int mods);
	void OnScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
};

#endif
