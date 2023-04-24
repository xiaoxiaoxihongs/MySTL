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
		// û���쳣���� 
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

	// ��first��ʼ���n��x
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
		// ����algobase���㷨���п�������Ϊ��POD����
		return copy(first, last, result);
	}

	template<class ForwardIterator, class InputIterator>
	inline ForwardIterator
		__uninitialized_copy_aux(InputIterator first, InputIterator last,
			ForwardIterator result, __false_type)
	{
		// �Լ�ʵ�֣���ʡ�����쳣����
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

	// ��[first, last)�����������䵽result֮��
	template<class ForwardIterator, class InputIterator>
	inline ForwardIterator 
		uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
	{
		return __uninitialized_copy(first, last, result, VALUE_TYPE(x));
	}

	// char�ػ�����[first, last)�����������䵽result֮��
	inline char* uninitialized_copy(const char* first, const char* last, char* result)
	{
		memmove(result, first, (last - first));
		return result + (last - first);
	}

	// wchar_t�ػ�����[first, last)�����������䵽result֮��
	inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
	{
		memmove(result, first, sizeof(wchar_t) * (last - first));
		return result + (last - first);
	}
}
