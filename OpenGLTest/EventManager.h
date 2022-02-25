#ifndef EVENT_MANAGER
#define EVENT_MANAGER

#include <list>
#include <functional>

namespace EventManager
{
	extern std::list<std::function<void()>> OnMainLoop;
	extern std::list<std::function<void()>> OnExitApp;

	/// <summary>
	/// Call an event
	/// </summary>
	void Call(std::list<std::function<void()>>* eventList);
};

#endif
