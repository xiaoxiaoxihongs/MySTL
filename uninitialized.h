#pragma once
#include "type_traits.h"
#include "construct.h"
#include "algobase.h"


namespace MySTL
{
	// fill
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

	// 从first开始填充n个x
	template<class ForwardIterator, class size, class T>
	inline ForwardIterator
		uninitialized_fill_n(ForwardIterator first, size n, const T& x)
	{
		return __uninitialized_fill_n(first, n, VALUE_TYPE(x));
	}

	// copy
	template<class ForwardIterator, class InputIterator>
	inline ForwardIterator
		__uninitialized_copy_aux(InputIterator first, InputIterator last,
			ForwardIterator result, __true_type)
	{
		// 调用algobase的算法进行拷贝，因为是POD类型
		return copy(first, last, result);
	}

	template<class ForwardIterator, class InputIterator>
	inline ForwardIterator
		__uninitialized_copy_aux(InputIterator first, InputIterator last,
			ForwardIterator result, __false_type)
	{
		// 自己实现，但省略了异常处理
		ForwardIterator cur = result;
		for (; first != last; ++first, ++cur)
			construct(&*cur, *first);
		return cur;
	}

	template<class ForwardIterator, class InputIterator, class T>
	inline ForwardIterator
		__uninitialized_copy(InputIterator first, InputIterator last,
			ForwardIterator result, T*)
	{
		typedef typename __type_traits<T>::is_POD_type Is_POD;
		return __uninitialized_copy_aux(first, last, result, Is_POD());
	}

	// 将[first, last)区间的数据填充到result之后
	template<class ForwardIterator, class InputIterator>
	inline ForwardIterator 
		uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
	{
		return __uninitialized_copy(first, last, result, VALUE_TYPE(x));
	}

	// char特化，将[first, last)区间的数据填充到result之后
	inline char* uninitialized_copy(const char* first, const char* last, char* result)
	{
		memmove(result, first, (last - first));
		return result + (last - first);
	}

	// wchar_t特化，将[first, last)区间的数据填充到result之后
	inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
	{
		memmove(result, first, sizeof(wchar_t) * (last - first));
		return result + (last - first);
	}
}
