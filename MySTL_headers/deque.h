#pragma once

#include "alloc.h"
#include "algobase.h"
#include "construct.h"


namespace MySTL
{
	/*
	deque��һ��˫�򿪿ڵ��������Կռ䣬��û�������ĸ����Ϊ���Ƕ�̬���Էֶ������ռ���϶��ɡ�
	��ȻdequeҲ�ṩRamdon Access Iterator�������ĵ�������������Ƶġ�
	���漰������ʱ����ý�deque�����ĸ��Ƶ�vector�У���vector������ƻ�deque��
	deque����һ��һ�εĶ����ռ���ɣ�������������������Щ�ֶεĶ����ռ���ά�������ļ���
	*/

	// ���е�deque�û�����ָ���ڵ��С����Υ����C++��׼�� (it can be detected using template template parameters)
	// ���Ա�ģ���ģ�������⵽�������Ѿ���ɾ��
	//template<class T, class Alloc = alloc, size_t BufSize = 0>
	//class deque {};


	// ��Ԫ�ش�СС��512ʱ������512/size,����ʱ����1
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

		// �������Զ���ģ�������Ҫ����׫д����������ͱ�
		using iterator_category = random_access_iterator_tag;
		using value_type		= T;
		using pointer			= Ptr;
		using reference			= Ref;
		using size_type			= size_t;
		using difference_type	= ptrdiff_t;

		// �����mapӦ����ָ���п�����ָ�룬�п�����deque��˵��
		using map_pointer		= T**;
		using self				= _Deque_iterator;

		// ������Ϊ�˱���������������

		// �˵�����ָ�򻺳�����currentԪ��
		T* cur;
		// ָ�򻺳�����ͷ
		T* first;
		// ָ�򻺳�����β
		T* last;
		// ָ���п���
		map_pointer node;

		// ���캯����Ĭ�Ϲ��졢ֵ���졢��������
		_Deque_iterator() :cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}
		_Deque_iterator(T* x, map_pointer y) :
			cur(x), first(*y), last(*y + buffer_size()), node(y) {}
		_Deque_iterator(const iterator& x):
			cur(x.cur), first(x.first), last(x.last), node(x.node) {}

		// �����������
		reference operator*()const { return *cur; }
		pointer operator->()const { return &(operator*()); }

		// ��ֵ�ļ��㷽���ǽ���������С��������������֮��Ļ���������
		// Ȼ�󽫵�������ƫ������ӵ�����ԵĻ�������,���� 01 2 3 4567 8 9 ab
		// ��������СΪ4����������ָ��ڶ���Ԫ�أ���һ��ָ�������Ԫ�أ�����3�ţ���һ����һ�ţ��м����2��
		// ��ô����֮��ľ���Ϊ4*(3-1-1) + (1-0) + (3-2) = 6;
		difference_type operator-(const self& x) const 
		{
			return difference_type(buffer_size()) * (node - x.node - 1) + (cur - first) +
				(x.last - x.cur);
		}

		self& operator++() 
		{
			++cur;
			// ����ǰ����������û�����ĩ��ʱ��ָ����һ��������
			if (cur == last) 
			{
				_set_node(node + 1);
				cur = first;
			}
			return *this;
		}

		// ����++��const�ķ����ܹ���ֹ++++������������
		const self& operator++(int)
		{
			self temp = *this;
			++ *this;
			return temp;
		}

		self& operator--() 
		{
			// �˴�Ҫ���ж��Ƿ���ͷ�ڵ㣬Ҫ����ͷ�ڵ�֮������ܻ����δ������Ϊ
			if (cur == first) 
			{
				_set_node(node - 1);
				cur = last;
			}
			--cur;
			return *this;
		}
		
		// --*this��һ�������Լ�����������ص���һ���������ö������µĶ���
		// �����������������ȷ��ص�ǰ��������ֵ��Ȼ�󽫵�ǰ��������ǰ�ƶ�һ��λ��
		// ��Ϊ������ı䵱ǰ��������ͣ�ֻ��ı�����ֵ
		const self& operator--(int)
		{
			self temp = *this;
			-- *this;
			return temp;
		}

		// ʵ�������ȡ
		self& operator+=(difference_type n) 
		{
			// ����cur-first��ͱ�ɴӼ��ŵ������Ŀ�ͷ��ʼ���㣬����
			difference_type offset = n + (cur - first);
			if (offset >= 0 && offset < difference_type(buffer_size()))
				// ��Ŀ����ͬһ��������ʱ
				cur += n;
			else
			{
				// ������ͬһ��������ʱ,����ǰ���ƶ����Ǻ����ƶ��Ļ�����
				difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size()) :
					-difference_type((-offset - 1) / buffer_size()) - 1;
				_set_node(node_offset);
				cur = first + (offset - node_offset * difference_type(buffer_size()));
			}
			return *this;
		}

		// ����+=ʵ��
		// ����������һ��������Ա�����������޸ĵ�ǰ����״̬
		// ��temp���������Լ�����ı�ԭ��������ֵ
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