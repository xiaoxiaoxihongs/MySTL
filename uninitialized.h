#pragma once
#include "type_traits.h"
#include "construct.h"
#include "algobase.h"

namespace MySTL
{
	template<class ForwardIterator, class size, class T>
	inline ForwardIterator
		__uninitialized_fill_n_aux(ForwardIterator first, size n, const T& x, __true_type)
	{
		return fill_n(first, n, x);
	}

	template<class ForwardIterator, class size, class T>
	inline ForwardIterator
		__uninitialized_fill_n_aux(ForwardIterator first, size n, const T& x, __false_type)
	{
		ForwardIterator cur = first;
		// 没有异常处理 
		for (; n > 0; --n, ++cur) construct(&*cur, x);
		return cur;
	}


	template<class ForwardIterator, class size, class T, class Tp>
	inline ForwardIterator
		__uninitialized_fill_n(ForwardIterator first, size n, const T& x, Tp*)
	{
		typedef typename __type_traits<Tp>::is_POD_type Is_POD;
		return __uninitialized_fill_n_aux(first, n, x, Is_POD());
	}

	template<class ForwardIterator, class size, class T>
	inline ForwardIterator
		uninitialized_fill_n(ForwardIterator first, size n, const T& x)
	{
		return __uninitialized_fill_n(first, n, Value_type(x));
	}



}
