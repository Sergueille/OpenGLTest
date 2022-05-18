#pragma once

#include "TweenAction.h"
#include "LinkedList.h"

// NOTE: no cpp file because of template
// Tween type enum in TweenAction.h

template <typename T>
class TweenManager
{
public:
	static LinkedListElement<TweenAction<T>>* Tween(T startVal, T endVal, float duration, std::function<void(T value)> func, EaseType type);
	static void Cancel(LinkedListElement<TweenAction<T>>* action);

private:
	static LinkedList<TweenAction<T>> activeActions;

	static TweenManager<T>* instance;

	void OnMainLoop();
};

template <typename T>
LinkedList<TweenAction<T>> TweenManager<T>::activeActions = LinkedList<TweenAction<T>>();

template <typename T>
TweenManager<T>* TweenManager<T>::instance = nullptr;

template <typename T>
LinkedListElement<TweenAction<T>>* TweenManager<T>::Tween(T startVal, T endVal, float duration, std::function<void(T value)> func, EaseType type)
{
	if (TweenManager<T>::instance == nullptr)
	{
		TweenManager<T>::instance = new TweenManager<T>();
		EventManager::OnMainLoop.push_end([] { TweenManager<T>::instance->OnMainLoop(); });
	}

	TweenAction<T> newAction = TweenAction<T>(startVal, endVal, Utility::time, duration, func, type);
	return TweenManager<T>::activeActions.push_end(newAction);
}

template <typename T>
void TweenManager<T>::Cancel(LinkedListElement<TweenAction<T>>* action)
{
	activeActions.remove(action);
}

template <typename T>
void TweenManager<T>::OnMainLoop()
{
	for (auto el = activeActions.first; el != nullptr;)
	{
		if (el->value.IsFinshedAt(Utility::time)) // Action ended
		{
			auto next = el->next;
			activeActions.remove(el);
			el = next;
		}
		else
		{
			el->value.func(el->value.EvalAt(Utility::time));
			el = el->next;
		}
	}
}
