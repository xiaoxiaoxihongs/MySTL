#pragma once
#include <new.h>
#include "iterator_base.h"
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
	inline void destroy(T* p)
	{
		// 调用T类的析构函数
		p->~T();
	}


	// 如果元素的数值型别(value type)有non-trivial destructor
	template<class ForwardIterator>
	inline void __destroy_aux(ForwardIterator first, ForwardIterator end, __false_type)
	{
		for (; first < end; ++first)
		{
			destroy(&*first);
		}
	}

	// 如果元素的数值型别(value type)有trivial destructor
	template<class ForwardIterator>
	inline void __destroy_aux(ForwardIterator first, ForwardIterator end, __true_type ) {}

	template<class ForwardIterator, class T>
	inline void __destroy(ForwardIterator first, ForwardIterator end, T*)
	{
		// typedef创建了存在类型的别名，typename告诉编译器后面的是一个类型而不是一个成员
		typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
		__destroy_aux(first, end, trivial_destructor());
	}

	// 接受两个迭代器，并设法找出元素的数值型别，进而利用__type_traits<>求出最当措施
	template<class ForwardIterator>
	inline void destroy(ForwardIterator first, ForwardIterator end)
	{
		// Value_type函数应该要在iterator里面定义，返回迭代器类型
		__destroy(first, end, VALUE_TYPE(first));
	}
}
