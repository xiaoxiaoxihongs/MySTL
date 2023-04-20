#pragma once
#include <new.h>
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
	inline void destory(T* p)
	{
		// ����T�����������
		p->~T();
	}

	// �������������������跨�ҳ�Ԫ�ص���ֵ�ͱ𣬽�������__type_traits<>������ʩ
	template<class ForwardIterator>
	inline void destory(ForwardIterator first, ForwardIterator end)
	{
		// __(����_)����������Ϊ˽�к�����һ��_�Ǳ������������������ֲ������Ա����������
		// �и����ʣ������value_type()��������Ǹ�ʲô������
		// �������Ӧ��Ҫ��iterator���涨�壬���ص���������
		__destory(first, end, Value_type(first));
	}

	template<class ForwardIterator, class T>
	inline void __destory(ForwardIterator first, ForwardIterator end, T*)
	{
		// typedef�����˴������͵ı�����typename���߱������������һ�����Ͷ�����һ����Ա
		typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
		__destory_aux(first, end, trivial_destructor());
	}

	// ���Ԫ�ص���ֵ�ͱ�(value type)��non-trivial destructor
	template<class ForwardIterator>
	inline void __destory_aux(ForwardIterator first, ForwardIterator end, __false_type)
	{
		for (; first < end; ++first)
		{
			destory(&*first);
		}
	}

	// ���Ԫ�ص���ֵ�ͱ�(value type)��trivial destructor
	template<class ForwardIterator>
	inline void __destory_aux(ForwardIterator first, ForwardIterator end, __true_type ) {}



}
