#include "EventManager.h"

#include <iostream>

namespace EventManager 
{
	std::list<std::function<void()>> OnMainLoop = std::list<std::function<void()>>();
	std::list<std::function<void()>> OnExitApp = std::list<std::function<void()>>();
	std::list<GLFWmousebuttonfun> OnClick = std::list<GLFWmousebuttonfun>();
	std::list<GLFWscrollfun> OnScroll = std::list<GLFWscrollfun>();
	std::list<GLFWcharfun> OnCharPressed = std::list<GLFWcharfun>();

	void SetupEvents()
	{
		glfwSetMouseButtonCallback(Utility::window, OnClickCallback);
		glfwSetScrollCallback(Utility::window, OnScrollCallback);
		glfwSetCharCallback(Utility::window, OnCharPressedCallback);
	}

	void EventManager::Call(std::list<std::function<void()>>* eventList)
	{
		for (auto& suscribedFunc : *eventList)
		{
			suscribedFunc();
		}
	}

	void EventManager::OnClickCallback(GLFWwindow* window, int button, int action, int mods)
	{
		for (auto fun : OnClick)
		{
			fun(window, button, action, mods);
		}
	}

	void EventManager::OnScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		for (auto fun : OnScroll)
		{
			fun(window, xOffset, yOffset);
		}
	}

	void OnCharPressedCallback(GLFWwindow* window, unsigned int codepoint)
	{
		for (auto fun : OnCharPressed)
		{
			fun(window, codepoint);
		}
	}
}
