#pragma once
#include "iterator_base.h"

namespace MySTL
{
	// ʹ�õ�����ȫ�����������ڵ��1��ʼʱ�����һ���ڵ�λ��i��ʱ����ô�������ӽڵ�λ��2i��
	// ���ӽڵ�λ��2i+1�������ӽڵ�λ��i/2���������0��ʼʱ�����ӽڵ�λ��2i+1�������ӽڵ�λ��2i+2��
	// ���ڵ�λ��(i-1)/2��

	template <typename RandomAccessIterator, typename Distance, typename T>
	void __push_heap(RandomAccessIterator first, Distance hole_index,
		Distance top_index, T value)
	{
		Distance parent = (hole_index - 1) / 2;
		while (hole_index > top_index && *(first + parent) < value)
		{
			*(first + hole_index) = *(first + parent);
			hole_index = parent;
			parent = (hole_index - 1) / 2;
		}
		*(first + hole_index) = value;
	}

	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare>
	inline void __push_heap_aux(RandomAccessIterator first, RandomAccessIterator last,
		Distance*, T*)
	{
		__push_heap(first, Distance((last - first) - 1), Distance(0), T(*(last - 1)));
	}

	template <typename RandomAccessIterator>
	inline void push_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
		__push_heap_aux(first, last, DISTANCE_TYPE(first), VALUE_TYPE(first));
	}
}
