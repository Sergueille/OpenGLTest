#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include "Utility.h"
#include "LinkedList.h"

#include <list>
#include <functional>

using namespace Utility;

namespace EventManager
{
	extern LinkedList<std::function<void()>> OnMainLoop;
	extern LinkedList<std::function<void()>> OnExitApp;
	extern LinkedList<GLFWmousebuttonfun> OnClick;
	extern LinkedList<GLFWscrollfun> OnScroll;
	extern LinkedList<GLFWcharfun> OnCharPressed;
	extern LinkedList<std::function<void(GLFWwindow* window, int key, int scancode, int action, int mods)>> OnKeyPressed;

	extern LinkedList<std::function<void()>> OnOpenEditor;
	extern LinkedList<std::function<void()>> OnCloseEditor;

	extern LinkedList<std::function<void()>> nextFrameActions;

	void SetupEvents();

	/// <summary>
	/// Call an event
	/// </summary>
	void Call(LinkedList<std::function<void()>>* eventList);

	void OnClickCallback(GLFWwindow* window, int button, int action, int mods);
	void OnScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void OnCharPressedCallback(GLFWwindow* window, unsigned int codepoint);
	void OnKeyPressedCallbeck(GLFWwindow* window, int key, int scancode, int action, int mods);

	/// <summary>
	/// Do this action on the next game loop. The captured object must be still alive on next frame
	/// </summary>
	void DoInOneFrame(std::function<void()> func);
};

#endif
