#pragma once

#include <functional>

template <typename T>
class TweenAction
{
public:
	TweenAction<T>(T startVal, T endVal, float time, float duration, std::function<void(T value)> func);
	TweenAction<T>() {};

	T startVal = T();
	T endVal = T();

	float startTime = 0;
	float duration = 0;

	std::function<void(T value)> func;

	T EvalAt(float time);
	bool IsFinshedAt(float time);
};

template <typename T>
T TweenAction<T>::EvalAt(float time)
{
	return startVal + (endVal - startVal) * ((time - startTime) / duration);
}

template <typename T>
bool TweenAction<T>::IsFinshedAt(float time)
{
	return time > startTime + duration;
}

template<typename T>
TweenAction<T>::TweenAction(T startVal, T endVal, float time, float duration, std::function<void(T value)> func)
{
	this->startVal = startVal;
	this->endVal = endVal;
	this->startTime = time;
	this->duration = duration;
	this->func = func;
}