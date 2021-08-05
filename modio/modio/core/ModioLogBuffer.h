#pragma once
#include <deque>

/// @brief Internal buffer of the most recent log messages
class LogBuffer
{
	std::deque<std::string> elements;

public:
	void Insert(std::string Item)
	{
		elements.push_back(Item);
		while (elements.size() > 10)
		{
			elements.pop_front();
		}
	}
	void Clear()
	{
		elements.clear();
	}

	auto begin() const
	{
		return elements.begin();
	}

	auto end() const
	{
		return elements.end();
	}
};