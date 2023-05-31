#pragma once
#include "iterator_base.h"

namespace MySTL
{
	// 使用的是完全二叉树，当节点从1开始时，如果一个节点位于i处时，那么它的左子节点位于2i处
	// 右子节点位于2i+1处，父子节点位于i/2处，如果从0开始时，左子节点位于2i+1处，右子节点位于2i+2处
	// 父节点位于(i-1)/2处

	// 不支持自定义比较的
	template <typename RandomAccessIterator, typename Distance, typename T>
	void __push_heap(RandomAccessIterator first, Distance hole_index,
		Distance top_index, T value)
	{
		// 上溯操作，将新节点拿来与父节点比较，如果key比父节点大，那么就父子对调，直到不需要调换为止
		// 父节点
		Distance parent = (hole_index - 1) / 2;
		// 当未达到顶点，且父节点小于新值的时候
		while (hole_index > top_index && *(first + parent) < value)
		{
			// 调整洞值，先令洞值为父，再调整洞号令其向上提升为父节点，最后计算新父节点
			*(first + hole_index) = *(first + parent);
			hole_index = parent;
			parent = (hole_index - 1) / 2;
		}// 直到满足heap的次序特征
		// 令洞值为新值完成插入
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


	// 支持自定义比较的
	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare>
	void __push_heap_aux(RandomAccessIterator first,  Distance hole_index, Distance top_index, T value, Compare compare)
	{
		Distance parent = (hole_index - 1) / 2;
		while (hole_index > top_index && compare(*(first + parent), value))
		{
			*(first + hole_index) = *(first + parent);
			hole_index = parent;
			parent = (hole_index - 1) / 2;
		}
		*(first + hole_index) = value;
	}

	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare>
	inline void __push_heap(RandomAccessIterator first, RandomAccessIterator last, Distance*, T*, Compare compare)
	{
		__push_heap_aux(first, Distance((last - first) - 1), Distance(0), T(*(last - 1)), compare);
	}

	template <typename RandomAccessIterator, typename Compare>
	inline void push_heap(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
		__push_heap(first, last, DISTANCE_TYPE(first), VALUE_TYPE(first));
	}

	// pop_heap
	// 身为一个max-heap，那么它的最大值必然在根节点，pop就是取走根节点，然后调整树
	// 执行下溯操作，将空间节点与其最大子节点对调并下放，直到叶节点为止，然后将前述的被割舍的元素值设给
	// 这个已到达叶层节点的空洞，再执行一次上溯。pop_heap后最大元素置入容器尾部，并未取出

	template <typename RandomAccessIterator, typename T, typename Distance>
	void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value)
	{
		Distance topIndex = holeIndex;
		Distance rightChild = 2 * topIndex + 2;

		while (rightChild < len)
		{
			// 比较洞节点的两个子值，然后用right代表较大的值
			if (*(first + rightChild) < *(first + (rightChild - 1)))
				rightChild--;
			// 令较大的子值为洞，然后将洞下移至较大子节点处
			*(first + holeIndex) = *(first + rightChild);
			holeIndex = rightChild;
			// 新洞节点的右子节点
			rightChild = 2 * (rightChild - 1);
		}
		// 当没有右子节点，只有左子节点时
		if (rightChild == len)
		{
			// 令左子值为洞值，在令洞号下移至左子节点处
			*(first + holeIndex) = *(first + (rightChild - 1));
			holeIndex = rightChild - 1;
		}
		__push_heap(first, holeIndex, topIndex, value);
	}

	template <typename RandomAccessIterator, typename T, typename Distance>
	inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
		RandomAccessIterator result, T value, Distance*)
	{
		// 设定尾值为首值，于是尾值即为欲求结果，可以由客户端稍后再以pop_back()取出尾值
		*result = *first;
		// 洞号为树根即0处，欲调整值为value即原尾值
		__adjust_heap(first, Distance(0), Distance(last - first), value);
	}

	template <typename RandomAccessIterator, typename T>
	inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*)
	{
		// pop的结果应为底部容器的第一个元素，首先设定欲调整值为尾值，
		// 然后将首值调至尾节点(所以将以上迭代器result设置为last-1，然后重整[first, last-1))
		__pop_heap(first, last - 1, last - 1, T(*(last - 1)), DISTANCE_TYPE(first));
	}
	
	template <typename RandomAccessIterator>
	inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
		// 此处函数被调用时，新元素已经置于容器的尾端
		__pop_heap_aux(first, last, VALUE_TYPE(first));
	}
}
