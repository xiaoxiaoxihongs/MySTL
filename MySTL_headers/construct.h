#pragma once
#include <new.h>
#include "iterator_base.h"
#include "type_traits.h"

namespace MySTL
{
	template<class T1, class T2>
	inline void construct(T1* p, const T2& value)
	{
		// ����һ��ָ���һ��ֵ������ʼֵ�趨��ָ����ָ�Ŀռ���
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
		// ����T�����������
		p->~T();
	}


	// ���Ԫ�ص���ֵ�ͱ�(value type)��non-trivial destructor
	template<class ForwardIterator>
	inline void __destroy_aux(ForwardIterator first, ForwardIterator end, __false_type)
	{
		for (; first < end; ++first)
		{
			destroy(&*first);
		}
	}

	// ���Ԫ�ص���ֵ�ͱ�(value type)��trivial destructor
	template<class ForwardIterator>
	inline void __destroy_aux(ForwardIterator first, ForwardIterator end, __true_type ) {}

	template<class ForwardIterator, class T>
	inline void __destroy(ForwardIterator first, ForwardIterator end, T*)
	{
		// typedef�����˴������͵ı�����typename���߱������������һ�����Ͷ�����һ����Ա
		typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
		__destroy_aux(first, end, trivial_destructor());
	}

	// �������������������跨�ҳ�Ԫ�ص���ֵ�ͱ𣬽�������__type_traits<>������ʩ
	template<class ForwardIterator>
	inline void destroy(ForwardIterator first, ForwardIterator end)
	{
		// Value_type����Ӧ��Ҫ��iterator���涨�壬���ص���������
		__destroy(first, end, VALUE_TYPE(first));
	}
}
