#pragma once

#include "alloc.h"
#include "algobase.h"
#include "construct.h"


namespace MySTL
{
	/*
	deque是一种双向开口的连续线性空间，它没有容量的概念，因为它是动态的以分段连续空间组合而成。
	虽然deque也提供Ramdon Access Iterator，但它的迭代器是特殊设计的。
	当涉及到运算时，最好将deque完整的复制到vector中，将vector排序后复制回deque。
	deque是由一段一段的定量空间组成，它的最大任务就是在这些分段的定量空间上维护连续的假象。
	*/

	// 书中的deque用户可以指定节点大小，但违反了C++标准？ (it can be detected using template template parameters)
	// 可以被模板的模板参数检测到，所以已经被删除
	//template<class T, class Alloc = alloc, size_t BufSize = 0>
	//class deque {};


	// 当元素大小小于512时，传回512/size,大于时传回1
	inline size_t __deque_buf_size(size_t size) { return size < 512 ? size_t(512 / size) : size_t(1); }

	template<class T, class Ref, class Ptr>
	struct _Deque_iterator
	{
		using iterator			= _Deque_iterator<T, T&, T*>;
		using const_iterator	= _Deque_iterator<T, const T&, const T*>;

		static size_t buffer_size() { return __deque_buf_size(sizeof(T)); }
		void _set_node(pointer new_node) 
		{
			node = new_node;
			first = *new_node;
			last = first + difference_type(buffer_size());
		}

		// 迭代器自定义的，所以需要自行撰写五个迭代器型别
		using iterator_category = random_access_iterator_tag;
		using value_type		= T;
		using pointer			= Ptr;
		using reference			= Ref;
		using size_type			= size_t;
		using difference_type	= ptrdiff_t;

		// 这里的map应该是指向中控器的指针，中控器在deque中说明
		using map_pointer		= T**;
		using self				= _Deque_iterator;

		// 以下是为了保持与容器的联接

		// 此迭代器指向缓冲区的current元素
		T* cur;
		// 指向缓冲区的头
		T* first;
		// 指向缓冲区的尾
		T* last;
		// 指向中控器
		map_pointer node;

		// 构造函数区默认构造、值构造、拷贝构造
		_Deque_iterator() :cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}
		_Deque_iterator(T* x, map_pointer y) :
			cur(x), first(*y), last(*y + buffer_size()), node(y) {}
		_Deque_iterator(const iterator& x):
			cur(x.cur), first(x.first), last(x.last), node(x.node) {}

		// 重载运算符区
		reference operator*()const { return *cur; }
		pointer operator->()const { return &(operator*()); }

		// 差值的计算方法是将缓冲区大小乘以两个迭代器之间的缓冲区数，
		// 然后将迭代器的偏移量添加到其各自的缓冲区内,例如 01 2 3 4567 8 9 ab
		// 缓冲区大小为4，本迭代器指向第二个元素，另一个指向第三个元素，本在3号，另一个在一号，中间隔了2号
		// 那么他们之间的距离为4*(3-1-1) + (1-0) + (3-2) = 6;
		difference_type operator-(const self& x) const 
		{
			return difference_type(buffer_size()) * (node - x.node - 1) + (cur - first) +
				(x.last - x.cur);
		}

		self& operator++() 
		{
			++cur;
			// 当当前迭代器到达该缓冲区末端时，指向下一个缓冲区
			if (cur == last) 
			{
				_set_node(node + 1);
				cur = first;
			}
			return *this;
		}

		// 后置++，const的返回能够阻止++++这样的语句出现
		const self& operator++(int)
		{
			self temp = *this;
			++ *this;
			return temp;
		}

		self& operator--() 
		{
			// 此处要先判断是否在头节点，要是在头节点之间减可能会出现未定义行为
			if (cur == first) 
			{
				_set_node(node - 1);
				cur = last;
			}
			--cur;
			return *this;
		}
		
		// --*this是一个后置自减运算符，返回的是一个常量引用而不是新的对象
		// 这个运算符的作用是先返回当前迭代器的值，然后将当前迭代器向前移动一个位置
		// 因为它不会改变当前对象的类型，只会改变它的值
		const self& operator--(int)
		{
			self temp = *this;
			-- *this;
			return temp;
		}

		// 实现随机存取
		self& operator+=(difference_type n) 
		{
			// 加上cur-first后就变成从几号迭代器的开头开始计算，方便
			difference_type offset = n + (cur - first);
			if (offset >= 0 && offset < difference_type(buffer_size()))
				// 当目标在同一个缓冲区时
				cur += n;
			else
			{
				// 当不在同一个缓冲区时,计算前向移动或是后向移动的缓冲区
				difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size()) :
					-difference_type((-offset - 1) / buffer_size()) - 1;
				_set_node(node_offset);
				cur = first + (offset - node_offset * difference_type(buffer_size()));
			}
			return *this;
		}

		// 依托+=实现
		// 这个运算符是一个常量成员函数，不能修改当前对象状态
		// 对temp进行自增自减不会改变原迭代器的值
		self& operator+(difference_type n) const
		{
			self temp = *this;
			return temp += n;
		}

		self& operator-=(difference_type n)
		{

			return *this += -n;
		}

		self& operator-(difference_type n)
		{
			self temp = *this;
			return temp -= n;
		}

		self& operator[](difference_type n) const
		{
			return *(*this + n);
		}

		bool operator==(const self& x) const
		{
			return cur == x.cur;
		}

		bool operator!=(const self& x) const
		{
			return !(*this == x);
		}
		bool operator<(const self& x) const
		{
			return (node == x.node) ? (cur < x.cur) : (node < x.node);
		}
	};



	template<class T, class Alloc = alloc>
	class _Deque_base
	{
	public:

	};
}