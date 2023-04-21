#pragma once

namespace MySTL
{
	template <class OutputIterator, class size, class T>
	OutputIterator fill_n(OutputIterator first, size n, const T& value)
	{
		for (; n > 0; --n, ++first) *first = value;
		return first;
	}

}
