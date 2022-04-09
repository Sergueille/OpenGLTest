#pragma once

namespace Utility
{
	template <typename T>
	struct LinkedListElement;

	template <typename T>
	class LinkedList
	{
	public:
		LinkedList();
		~LinkedList();

		LinkedListElement<T>* first = nullptr;
		LinkedListElement<T>* last = nullptr;
		int length = 0;

		LinkedListElement<T>* push_end(T value);
		LinkedListElement<T>* push_start(T value);

		void remove(LinkedListElement<T>* value);
	};

	template <typename T>
	struct LinkedListElement {
		T value;
		LinkedListElement* next;
		LinkedListElement* prev;
	};

	///////////////////////////// IMPLEMENTATION

	template<typename T>
	inline LinkedList<T>::LinkedList()
	{
	}

	template<typename T>
	LinkedList<T>::~LinkedList()
	{
		if (first != nullptr)
		{
			LinkedListElement<T>* current = first;

			while (current->next != nullptr)
			{
				LinkedListElement<T>* next = current->next;
				delete current;
				current = next;
			}

			delete current;
		}
	}

	template<typename T>
	inline LinkedListElement<T>* LinkedList<T>::push_end(T value)
	{
		auto newEl = new LinkedListElement<T>();
		newEl->value = value;

		if (length == 0)
		{
			first = newEl;
			last = newEl;
		}
		else
		{
			newEl->prev = last;
			last->next = newEl;
			last = newEl;
		}

		length++;
		return newEl;
	}

	template<typename T>
	LinkedListElement<T>* LinkedList<T>::push_start(T value)
	{
		auto newEl = new LinkedListElement<T>();
		newEl->value = value;

		if (length == 0)
		{
			first = newEl;
			last = newEl;
		}
		else
		{
			newEl->next = first;
			first->prev = newEl;
			first = newEl;
		}

		length++;
		return newEl;
	}

	template<typename T>
	void LinkedList<T>::remove(LinkedListElement<T>* value)
	{
		if (value->prev != nullptr)
			value->prev->next = value->next;

		if (value->next != nullptr)
			value->next->prev = value->prev;

		if (length == 1)
		{
			last = nullptr;
			first = nullptr;
		}

		if (value == last)
			last = value->prev;

		if (value == first)
			first = value->next;

		length--;
		delete value;
	}

}
