#pragma once
#include "iterator_base.h"

namespace MySTL
{
	// ʹ�õ�����ȫ�����������ڵ��1��ʼʱ�����һ���ڵ�λ��i��ʱ����ô�������ӽڵ�λ��2i��
	// ���ӽڵ�λ��2i+1�������ӽڵ�λ��i/2���������0��ʼʱ�����ӽڵ�λ��2i+1�������ӽڵ�λ��2i+2��
	// ���ڵ�λ��(i-1)/2��

	// ��֧���Զ���Ƚϵ�
	template <typename RandomAccessIterator, typename Distance, typename T>
	void __push_heap(RandomAccessIterator first, Distance hole_index,
		Distance top_index, T value)
	{
		// ���ݲ��������½ڵ������븸�ڵ�Ƚϣ����key�ȸ��ڵ����ô�͸��ӶԵ���ֱ������Ҫ����Ϊֹ
		// ���ڵ�
		Distance parent = (hole_index - 1) / 2;
		// ��δ�ﵽ���㣬�Ҹ��ڵ�С����ֵ��ʱ��
		while (hole_index > top_index && *(first + parent) < value)
		{
			// ������ֵ�����ֵΪ�����ٵ�������������������Ϊ���ڵ㣬�������¸��ڵ�
			*(first + hole_index) = *(first + parent);
			hole_index = parent;
			parent = (hole_index - 1) / 2;
		}// ֱ������heap�Ĵ�������
		// �ֵΪ��ֵ��ɲ���
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


	// ֧���Զ���Ƚϵ�
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
	// ��Ϊһ��max-heap����ô�������ֵ��Ȼ�ڸ��ڵ㣬pop����ȡ�߸��ڵ㣬Ȼ�������
	// ִ�����ݲ��������ռ�ڵ���������ӽڵ�Ե����·ţ�ֱ��Ҷ�ڵ�Ϊֹ��Ȼ��ǰ���ı������Ԫ��ֵ���
	// ����ѵ���Ҷ��ڵ�Ŀն�����ִ��һ�����ݡ�pop_heap�����Ԫ����������β������δȡ��

	template <typename RandomAccessIterator, typename T, typename Distance>
	void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value)
	{
		Distance topIndex = holeIndex;
		Distance rightChild = 2 * topIndex + 2;

		while (rightChild < len)
		{
			// �Ƚ϶��ڵ��������ֵ��Ȼ����right����ϴ��ֵ
			if (*(first + rightChild) < *(first + (rightChild - 1)))
				rightChild--;
			// ��ϴ����ֵΪ����Ȼ�󽫶��������ϴ��ӽڵ㴦
			*(first + holeIndex) = *(first + rightChild);
			holeIndex = rightChild;
			// �¶��ڵ�����ӽڵ�
			rightChild = 2 * (rightChild - 1);
		}
		// ��û�����ӽڵ㣬ֻ�����ӽڵ�ʱ
		if (rightChild == len)
		{
			// ������ֵΪ��ֵ����������������ӽڵ㴦
			*(first + holeIndex) = *(first + (rightChild - 1));
			holeIndex = rightChild - 1;
		}
		__push_heap(first, holeIndex, topIndex, value);
	}

	template <typename RandomAccessIterator, typename T, typename Distance>
	inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
		RandomAccessIterator result, T value, Distance*)
	{
		// �趨βֵΪ��ֵ������βֵ��Ϊ�������������ɿͻ����Ժ�����pop_back()ȡ��βֵ
		*result = *first;
		// ����Ϊ������0����������ֵΪvalue��ԭβֵ
		__adjust_heap(first, Distance(0), Distance(last - first), value);
	}

	template <typename RandomAccessIterator, typename T>
	inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*)
	{
		// pop�Ľ��ӦΪ�ײ������ĵ�һ��Ԫ�أ������趨������ֵΪβֵ��
		// Ȼ����ֵ����β�ڵ�(���Խ����ϵ�����result����Ϊlast-1��Ȼ������[first, last-1))
		__pop_heap(first, last - 1, last - 1, T(*(last - 1)), DISTANCE_TYPE(first));
	}
	
	template <typename RandomAccessIterator>
	inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
		// �˴�����������ʱ����Ԫ���Ѿ�����������β��
		__pop_heap_aux(first, last, VALUE_TYPE(first));
	}
}
