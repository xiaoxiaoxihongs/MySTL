#pragma once
#include "alloc.h"
#include "uninitialized.h"


namespace MySTL
{
	/*
	vector维护一个连续线性的空间，无论是什么元素，普通指针都可以当作vector的迭代器使用(vector迭代器的所有操作普通指针都具备)
	vector提供的是Random Access Iterators
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
		// 此处是二级配置器
		typedef simple_alloc<value_type, alloc> data_allocator;

		// 目前使用的空间的头
		iterator start;
		// 目前使用的空间的尾
		iterator finish;
		// 目前可使用的空间的尾
		iterator end_of_storage;

		void insert_aux(iterator position, const T& x);
		void deallocate()
		{
			// 当释放内存时，调用配置器的dellocate
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
		// 首尾判断
		iterator begin() { return start; }
		const_iterator begin() const { return start; }
		iterator end() { return finish; }
		const_iterator end() const { return finish; }

		// 大小,强制转化陈size_type
		size_type size() { return size_type(end() - begin()); }



	};
}
	