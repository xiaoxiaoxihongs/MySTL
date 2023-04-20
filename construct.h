#pragma once
#include <new.h>
#include "type_traits.h"

namespace MySTL
{
	template<class T1, class T2>
	inline void construct(T1* p, const T2& value)
	{
		// 接受一个指针和一个值，将初始值设定到指针所指的空间上
		new((void*)p) T1(value);
	}

	template<class T1>
	inline void construct(T1* p)
	{
		new ((void*)p) T1();
	}

	template<class T>
	inline void destory(T* p)
	{
		// 调用T类的析构函数
		p->~T();
	}

	// 接受两个迭代器，并设法找出元素的数值型别，进而利用__type_traits<>求出最当措施
	template<class ForwardIterator>
	inline void destory(ForwardIterator first, ForwardIterator end)
	{
		// __(两个_)好像是声明为私有函数，一个_是保护函数，但这里面又不是类成员？？？不懂
		// 有个疑问：这里的value_type()这个函数是个什么逼玩意
		// 这个函数应该要在iterator里面定义，返回迭代器类型
		__destory(first, end, Value_type(first));
	}

	template<class ForwardIterator, class T>
	inline void __destory(ForwardIterator first, ForwardIterator end, T*)
	{
		// typedef创建了存在类型的别名，typename告诉编译器后面的是一个类型而不是一个成员
		typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
		__destory_aux(first, end, trivial_destructor());
	}

	// 如果元素的数值型别(value type)有non-trivial destructor
	template<class ForwardIterator>
	inline void __destory_aux(ForwardIterator first, ForwardIterator end, __false_type)
	{
		for (; first < end; ++first)
		{
			destory(&*first);
		}
	}

	// 如果元素的数值型别(value type)有trivial destructor
	template<class ForwardIterator>
	inline void __destory_aux(ForwardIterator first, ForwardIterator end, __true_type ) {}



}
