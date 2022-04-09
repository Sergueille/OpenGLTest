#include "EventManager.h"

#include <iostream>

namespace EventManager 
{
	LinkedList<std::function<void()>> OnMainLoop = LinkedList<std::function<void()>>();
	LinkedList<std::function<void()>> OnExitApp = LinkedList<std::function<void()>>();
	LinkedList<GLFWmousebuttonfun> OnClick = LinkedList<GLFWmousebuttonfun>();
	LinkedList<GLFWscrollfun> OnScroll = LinkedList<GLFWscrollfun>();
	LinkedList<GLFWcharfun> OnCharPressed = LinkedList<GLFWcharfun>();

	LinkedList<std::function<void()>> OnOpenEditor = LinkedList<std::function<void()>>();
	LinkedList<std::function<void()>> OnCloseEditor = LinkedList<std::function<void()>>();

	void SetupEvents()
	{
		glfwSetMouseButtonCallback(Utility::window, OnClickCallback);
		glfwSetScrollCallback(Utility::window, OnScrollCallback);
		glfwSetCharCallback(Utility::window, OnCharPressedCallback);
	}

	void EventManager::Call(LinkedList<std::function<void()>>* eventList)
	{
		for (auto el = eventList->first; el != nullptr; el = el->next)
		{
			el->value();
		}
	}

	void EventManager::OnClickCallback(GLFWwindow* window, int button, int action, int mods)
	{
		for (auto el = OnClick.first; el != nullptr; el = el->next)
		{
			el->value(window, button, action, mods);
		}
	}

	void EventManager::OnScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		for (auto el = OnScroll.first; el != nullptr; el = el->next)
		{
			el->value(window, xOffset, yOffset);
		}
	}

	void OnCharPressedCallback(GLFWwindow* window, unsigned int codepoint)
	{
		for (auto el = OnCharPressed.first; el != nullptr; el = el->next)
		{
			el->value(window, codepoint);
		}
	}
}
