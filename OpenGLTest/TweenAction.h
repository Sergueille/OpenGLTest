#pragma once

#include <functional>

enum EaseType
{
	linear,
	sineIn, sineOut, sineInOut,
	quadIn, quadOut, quadInOut,
	cubicIn, cubicOut, cubicInOut,
	quartIn, quartOut, quartInOut,
	quintIn, quintOut, quintInOut,
	expoIn, expoOut, expoInOut,
	circIn, circOut, circInOut,
};

template <typename T>
class TweenAction
{
public:
	TweenAction<T>(T startVal, T endVal, float time, float duration, std::function<void(T value)> func, EaseType easeType);
	TweenAction<T>() {};

	EaseType easeType = EaseType::linear;

	T startVal = T();
	T endVal = T();

	float startTime = 0;
	float duration = 0;

	std::function<void(T value)> func = nullptr;
	std::function<void()> onFinished = nullptr;
	std::function<bool()> conditionFunc = nullptr;

	LinkedListElement<TweenAction<T>>* listElement = nullptr;

	T EvalAt(float time);
	bool IsFinshedAt(float time);

	TweenAction<T>* SetOnFinished(std::function<void()> function);
	TweenAction<T>* SetCondition(std::function<bool()> function);
};

template <typename T>
T TweenAction<T>::EvalAt(float time)
{
	float t = ((time - startTime) / duration);
	float amount;

	switch (easeType)
	{
	case EaseType::sineIn:
		amount = 1.0f - (float)cos((t * PI) / 2);
		break;
	case EaseType::sineOut:
		amount = (float)sin((t * PI) / 2);
		break;
	case EaseType::sineInOut:
		amount = -((float)cos(PI * t) - 1) / 2;
		break;
	case EaseType::quadIn:
		amount = t * t;
		break;
	case EaseType::quadOut:
		amount = 1 - (1 - t) * (1 - t);
		break;
	case EaseType::quadInOut:
		amount = t < 0.5f ? 2.0f * t * t : 1 - (float)pow(-2 * t + 2, 2) / 2;
		break;
	case EaseType::cubicIn:
		amount = t * t * t;
		break;
	case EaseType::cubicOut:
		amount = 1 - (float)pow(1 - t, 3);
		break;
	case EaseType::cubicInOut:
		amount = t < 0.5f ? 4.0f * t * t * t : 1 - (float)pow(-2 * t + 2, 3) / 2;
		break;
	case EaseType::quartIn:
		amount = t * t * t * t;
		break;
	case EaseType::quartOut:
		amount = 1 - (float)pow(1 - t, 4);
		break;
	case EaseType::quartInOut:
		amount = t < 0.5f ? 8 * (float)pow(t, 4) : 1 - (float)pow(-2 * t + 2, 4) / 2;
		break;
	case EaseType::quintIn:
		amount = (float)pow(t, 5);
		break;
	case EaseType::quintOut:
		amount = 1 - (float)pow(1 - t, 5);
		break;
	case EaseType::quintInOut:
		amount = t < 0.5f ? 16 * (float)pow(t, 5) : 1 - (float)pow(-2 * t + 2, 5) / 2;
		break;
	case EaseType::expoIn:
		amount = t == 0 ? 0 : (float)pow(2, 10 * t - 10);
		break;
	case EaseType::expoOut:
		amount = t == 1 ? 1 : 1 - (float)pow(2, -10 * t);
		break;
	case EaseType::expoInOut:
		amount = t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? (float)pow(2, 20 * t - 10) / 2 : (2 - (float)pow(2, -20 * t + 10)) / 2;
		break;
	case EaseType::circIn:
		amount = 1 - (float)sqrt(1 - (float)pow(t, 2));
		break;
	case EaseType::circOut:
		amount = (float)sqrt(1 - (float)pow(t - 1, 2));
		break;
	case EaseType::circInOut:
		amount = t < 0.5f ? (1 - (float)sqrt(1 - (float)pow(2 * t, 2))) / 2 : ((float)sqrt(1 - (float)pow(-2 * t + 2, 2)) + 1) / 2;
		break;
	default:
		amount = t;
	};

	return startVal + (endVal - startVal) * (float)amount;
}

template <typename T>
bool TweenAction<T>::IsFinshedAt(float time)
{
	return time > startTime + duration;
}

template<typename T>
inline TweenAction<T>* TweenAction<T>::SetOnFinished(std::function<void()> function)
{
	onFinished = function;
	return this;
}

template<typename T>
inline TweenAction<T>* TweenAction<T>::SetCondition(std::function<bool()> function)
{
	conditionFunc = function;
	return this;
}

template<typename T>
TweenAction<T>::TweenAction(T startVal, T endVal, float time, float duration, std::function<void(T value)> func, EaseType easeType)
{
	this->startVal = startVal;
	this->endVal = endVal;
	this->startTime = time;
	this->duration = duration;
	this->func = func;
	this->easeType = easeType;
}
