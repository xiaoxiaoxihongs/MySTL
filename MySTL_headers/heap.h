#pragma once
#include "iterator_base.h"

namespace MySTL
{
	// 使用的是完全二叉树，当节点从1开始时，如果一个节点位于i处时，那么它的左子节点位于2i处
	// 右子节点位于2i+1处，父子节点位于i/2处，如果从0开始时，左子节点位于2i+1处，右子节点位于2i+2处
	// 父节点位于(i-1)/2处

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
