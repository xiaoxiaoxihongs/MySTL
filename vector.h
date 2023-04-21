#pragma once
#include "alloc.h"
#include "uninitialized.h"


namespace MySTL
{
	/*
	vectorά��һ���������ԵĿռ䣬������ʲôԪ�أ���ָͨ�붼���Ե���vector�ĵ�����ʹ��(vector�����������в�����ָͨ�붼�߱�)
	vector�ṩ����Random Access Iterators
	*/
	template<class T, class Alloc = alloc>
	class vector
	{
	public:
		typedef T					value_type;
		typedef value_type*			point;
		typedef const value_type*	const_point;
		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;
		typedef value_type&			reference;
		typedef const value_type&	const_reference;
		typedef size_t				size_type;
		typedef ptrdiff_t			difference_type;
	protected:
		// �˴��Ƕ���������
		typedef simple_alloc<value_type, alloc> data_allocator;

		// Ŀǰʹ�õĿռ��ͷ
		iterator start;
		// Ŀǰʹ�õĿռ��β
		iterator finish;
		// Ŀǰ��ʹ�õĿռ��β
		iterator end_of_storage;

		void insert_aux(iterator position, const T& x);
		void deallocate()
		{
			// ���ͷ��ڴ�ʱ��������������dellocate
			if (start) data_allocator::dellocate(start, end_of_storage - start);
		}

		void fill_initialize(size_type s, const T& value)
		{
			start = allocate_and_fill(n, value);
			finish = start + n;
			end_of_storage = finish;
		}
		
		iterator allocate_and_fill(size_type s, const T& x)
		{
			iterator result = data_allocator::allocate(s);
			uninitialized_fill_n(result, s, x);
			return result;
		}

	public:
		// ��β�ж�
		iterator begin() { return start; }
		const_iterator begin() const { return start; }
		iterator end() { return finish; }
		const_iterator end() const { return finish; }

		// ��С,ǿ��ת����size_type
		size_type size() { return size_type(end() - begin()); }



	};
}
	