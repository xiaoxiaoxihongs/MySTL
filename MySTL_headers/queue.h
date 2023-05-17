#pragma once
#include "deque.h"

namespace MySTL
{

	template<typename T, typename Sequence = deque<T>>
	class queue
	{
	private:
	public:
		using value_type		= typename Sequence::value_type;
		using size_type			= typename Sequence::size_type;
		using container_type	= Sequence;
		using reference			= typename Sequence::reference;
		using const_reference	= typename Sequence::const_reference;
	protected:
		Sequence c;
	public:
		bool empty() { return c.empty(); }
		size_type size() { return c.size(); }
		reference front() { return c.front(); }
		const_reference cfront() const { return c.cfront(); }
		reference back() { return c.back(); }
		const_reference cback() const { return c.cback(); }

		void push() { c.push_back(); }
		void pop() { c.pop_front(); }
	};


}
