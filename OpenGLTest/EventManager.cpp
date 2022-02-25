#include "EventManager.h"

#include <iostream>

namespace EventManager 
{
	std::list<std::function<void()>> OnMainLoop = std::list<std::function<void()>>();
	std::list<std::function<void()>> OnExitApp = std::list<std::function<void()>>();

	void EventManager::Call(std::list<std::function<void()>>* eventList)
	{
		for (auto& suscribedFunc : *eventList)
		{
			suscribedFunc();
		}
	}
}
