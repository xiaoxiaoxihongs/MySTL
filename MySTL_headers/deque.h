#pragma once

#include "alloc.h"
#include "uninitialized.h"


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

		bool operator==(const self& x) const { return cur == x.cur; }
		bool operator!=(const self& x) const { return !(*this == x); }
		bool operator< (const self& x) const { return (node == x.node) ? (cur < x.cur) : (node < x.node); }
		bool operator<=(const self& x) const { return !(x < *this); }
		bool operator> (const self& x) const { return x < *this; }
		bool operator>=(const self& x) const { return !(*this < x); }
	};

	template<class T, class Ref, class Ptr>
	inline _Deque_iterator<T, Ref, Ptr>
		operator+(ptrdiff_t n, const _Deque_iterator<T, Ref, Ptr> x)
	{
		return x + n;
	}

	template<class T, class Ref, class Ptr>
	inline random_access_iterator_tag
		iterator_category(const _Deque_iterator<T, Ref, Ptr>&)
	{
		return random_access_iterator_tag();
	}

	// 该函数返回一个指向T类型的指针，所以返回值并不重要，仅用于推导类型T的伪值
	// 用于编写可与不同类型迭代器一起使用的泛型代码
	template<class T, class Ref, class Ptr>
	inline T* value_type(const _Deque_iterator<T, Ref, Ptr>&)
	{
		return 0;
	}

	template<class T, class Ref, class Ptr>
	inline ptrdiff_t* distance_type(const _Deque_iterator<T, Ref, Ptr>&)
	{
		return 0;
	}

	template<class T, class Alloc = alloc>
	class _Deque_base
	{
	public:
		using iterator			= _Deque_iterator<T, T&, T*>;
		using const_iterator	= _Deque_iterator<T, const T&, const T*>;

		_Deque_base(size_t num_elements) :
			_map(0), _map_size(0), _start(), _finish()
		{
			_initialize_map(num_elements);
		}
		_Deque_base(const _Deque_base& base) :
			_map(base._map), _map_size(base._map_size), _start(base._start), _finish(base._finish) {}
		template<typename U>
		_Deque_base(_Deque_base<U>&& base) noexcept :
			_map(base._map), _map_size(base._map_size), _start(base._start), _finish(base._finish) 
		{
			base._map = nullptr;
			base._map_size = 0;
			base._start = iterator();
			base._finish = iterator();
		}
		_Deque_base() :_map(0), _map_size(0), _start(), _finish() {}
		~_Deque_base();
	protected:
		T** _map;
		size_t _map_size;
		iterator _start;
		iterator _finish;

		enum {_initial_map_size = 8};

		using _Node_alloc = simple_alloc<T, Alloc>;
		using _Map_alloc  = simple_alloc<T*, Alloc>;

	protected:
		void _initialize_map(size_t num_elements);
		void _create_nodes(T** _node_start, T** _node_finish);
		void _destroy_nodes(T** _node_start, T** _node_finish);
	protected:
		T* _allocate_node() { return _Node_alloc::allocate(__deque_buf_size(sizeof(T))); }
		void _deallocate_node(T* p) { return _Node_alloc::deallocate(p, __deque_buf_size(sizeof(T))); }
		T** _allocate_map(size_t n) { return _Map_alloc::allocate(n); }
		void _deallocate_map(T** p, size_t n) { return _Map_alloc::deallocate(p, n); }
	};

	template<class T, class Alloc>
	_Deque_base<T, Alloc>::~_Deque_base() 
	{
		if (_map)
		{
			_destroy_nodes(_start.node, _finish.node + 1);
			_deallocate_map(_map, _map_size);
		}
	}

	template<class T, class Alloc>
	inline void _Deque_base<T, Alloc>::_initialize_map(size_t num_elements)
	{
		// 需要的节点等于元素个数除以每个缓冲区可容纳的元素个数 +1，如果整除那么就会多配置一个节点
		size_t num_nodes = num_elements / __deque_buf_size(sizeof(T)) + 1;

		// 一个map要管理的节点最少是8个，最多的是所需节点数+2（用于头尾？）
		_map_size = max((size_t)_initial_map_size, num_nodes + 2);
		_map = _allocate_map(_map_size);

		// nstart和nfinish指向map拥有的全部节点的中央地区，能令两端扩充一样大，每个节点对应一个缓冲区
		T** nstart = _map + (_map_size - num_nodes) / 2;
		T** nfinish = nstart + num_nodes;

		try
		{
			_create_nodes(nstart, nfinish);
		}
		catch (...)
		{
			_deallocate_map(_map, _map_size);
			_map = 0, _map_size = 0;
			throw;
		}
		
		//将迭代器设定到正确的位置，当多配置一个节点时，finish的cur会指向它
		_start._set_node(nstart);
		_finish._set_node(nfinish);
		_start.cur = _start.first;
		_finish.cur = _finish.first + num_elements % __deque_buf_size(sizeof(T));

	}

	template<class T, class Alloc>
	inline void _Deque_base<T, Alloc>::_create_nodes(T** _node_start, T** _node_finish)
	{
		//for (T** cur = _node_start; cur < _node_finish; ++cur)
		//	*cur = _allocate_node();

		// 创建的时候尽量搞个异常处理，防止创建了却没有管理
		T** cur;
		try 
		{
			for (cur = _node_start; cur < _node_finish; ++cur)
				*cur = _allocate_node();
		}
		catch (...)
		{
			_destroy_nodes(_node_start, cur);
			throw;
		}
	}

	template<class T, class Alloc>
	void _Deque_base<T, Alloc>::_destroy_nodes(T** _node_start, T** _node_finish)
	{
		for (T** n = _node_start; n < _node_finish; ++n)
			_deallocate_node(*n);
	}

	template<class T, class Alloc = alloc>
	class deque :protected _Deque_base<T, Alloc>
	{
		using _Base = _Deque_base<T, Alloc>;
		// 经典内容
	public:
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using iterator = _Base::iterator;
		using const_iterator = _Base::const_iterator;

	protected:
		// 元素指针的指针
		using map_pointer = pointer*;
		static size_t _buffer_size() { return __deque_buf_size(sizeof(_Tp)); }
		void _fill_initialize(const value_type& value);

	// 还是写经典内容
	public:
		iterator begin() { return _start; }
		const_iterator cbegin() const { return _start; }
		iterator end() { return _finish; }
		const_iterator cend() const { return _finish; }

		reference operator[](size_type n)
		{
			return _start[difference_type(n)];
		}
		const_reference operator[](size_type n) const
		{
			return _start[difference_type(n)];
		}

		reference front() { return *_start; }
		const_reference cfront() const { return *_start; }
		reference back() 
		{
			iterator temp = _finish;
			--temp;
			return *temp;
		}
		const_reference cback() const
		{
			iterator temp = _finish;
			--temp;
			return *temp;
		}

		size_type size() const { return _finish - _start; }
		size_type max_size() const { return size_type(-1); }
		bool empty() const { return _finish == _start; }

	// 构造函数大军，写了移动拷贝函数，不知道对不对
	public:
		explicit deque() : _Base(0) {};
		deque(const deque& x) : _Base(x.size()) { uninitialized_copy(x.cbegin(), x.cend(), _start); }
		deque(size_type n, const value_type& value) :_Base(n) { _fill_initialize(value); }
		explicit deque(size_type n) : _Base(n) { _fill_initialize(value_type()); }
		deque(const value_type* first, const value_type* last) :_Base(last - first) { uninitialized_copy(first, last, _start); }
		deque(const iterator first, const iterator last) : _Base(last - first) { uninitialized_copy(first, last); }
		
		// 移动拷贝函数，使用一个通用引用来接收一个参数，使它能够绑定任何类型的值
		template<typename U>
		deque(deque<U>&& x) :_Base(std::forward<deque<U>>(x)) {}

		~deque() { destroy(_start, _finish); }

	public:
		deque& operator=(const deque& x)
		{
			const size_type len = size();
			if (&x != this)
			{
				if (len >= x.size())
					// 自身长度大于x的长度时，将x从_start位置开始拷贝，然后获得最后一个元素之后的迭代器
					// 再将多出的部分删除
					erase(copy(x.cbegin(), x.cend(), _start));
				else
				{
					// 自身长度小于那就先将自身长度的内容拷贝过去，然后在直接往后插入内容
					const_iterator mid = x.begin() + difference_type(len);
					copy(x.cbegin(), mid, _start);
					insert(_finish, mid, x.cend());
				}
			}
			return *this;
		}
		// 移动赋值运算符，不知道对不对
		deque& operator=(deque&& x) noexcept
		{
			swap(_map, x._map);
			swap(_map_size, x._map_size);
			swap(_start, x._start);
			swap(_finish, x._finish);
			return *this;
		}

		void swap(deque& x)
		{
			std::swap(_map, x._map);
			std::swap(_map_size, x._map_size);
			std::swap(_start, x._start);
			std::swap(_finish, x._finish);
		}

	// push and pop
	public:
		void fill_assign(size_type n, const T& value)
		{
			if (n > size())
			{
				fill(begin(), end(), value);
				insert(end(), n - size(), value);
			}
			else
			{
				erase(begin() + n, end());
				fill(begin, end(), value);
			}
		}

		void push_back(const value_type& t)
		{
			if (_finish.cur != _finish.last)
			{
				construct(_finish.cur, t);
				++_finish.cur;
			}
			else
				_push_back_aux(t);
		}

		void push_back()
		{
			if (_finish.cur != _finish.last)
			{
				construct(_finish.cur);
				++_finish.cur;
			}
			else
				_push_back_aux();
		}

		void push_front(const value_type& t)
		{
			if (_start.cur != _start.first)
			{
				construct(_start.cur - 1, t);
				--_start.cur;
			}
			else
				_push_front_aux(t);
		}

		void push_front()
		{
			if (_start.cur != _start.first)
			{
				construct(_start.cur - 1);
				--_start.cur;
			}
			else
				_push_front_aux();
		}

		void pop_back()
		{
			// 指向尾后元素的下一个，先移动再摧毁
			if (_finish.cur != _finish.first)
			{
				--_finish.cur;
				destroy(_finish.cur);
			}
			else
				_pop_back_aux();
		}

		void pop_front()
		{
			// 指向当前需要被摧毁的元素
			if (_start.cur != _start.last - 1)
			{
				destroy(_start.cur);
				++_start.cur;
			}
			else
				_pop_front_aux();
		}
		
	// insert
	public:
		iterator insert(iterator position, const value_type& x)
		{
			if (position.cur == _start.cur)
			{
				push_front(x);
				return _start;
			}
			else if (position.cur == _finish.cur)
			{
				push_back(x);
				//由于指向尾后的下一个
				iterator temp = _finish;
				--temp;
				return temp;
			}
			else
				return _insert_aux(position, x);
		}

		iterator insert(iterator position)
		{
			insert(position, value_type());
		}

		void insert(iterator position, size_type n, const value_type& x)
		{
			_fill_insert(position, n, x);
		}

		void insert(iterator position, const value_type* first, const value_type* last);

		void insert(iterator position, const_iterator first, const_iterator last);

	// erase
	public:
		// 删除点
		iterator erase(iterator position)
		{
			iterator next = position;
			++next;
			// 如果在清除点之前的元素比清除点之后的元素少，就移动清除点之前的元素覆盖，
			// 然后删除第一个冗余，反之亦然
			difference_type index = position - _start;
			if (size_type(index) < (this->size() >> 1))
			{
				copy_backward(_start, position, next);
				pop_front();
			}
			else
			{
				copy(next, _finish, position);
				pop_back();
			}
			return _start + index;
		}

		// 删除范围
		iterator erase(iterator first, iterator last);

		// 清空
		void clear();

	// resize
	public:
		void resize(size_type new_size, const value_type& x)
		{
			// 如果小于就擦除多的部分
			const size_type len = size();
			if (new_size < len)
			{
				erase(_start + new_size, _finish);
			}
			else
				insert(_finish, new_size - len, x);
		}

		void resize(size_type new_size) { resize(new_size, value_type()); }
	protected:
		void _fill_insert(iterator position, size_type n, const value_type& x);
		void _pop_front_aux();
		void _pop_back_aux();
		void _push_front_aux();
		void _push_front_aux(const value_type& t);
		void _push_back_aux();
		void _push_back_aux(const value_type& t);

		iterator _insert_aux(iterator position, const value_type& x);
		iterator _insert_aux(iterator position);
		void _insert_aux(iterator position, size_type n, const value_type& x);
		void _insert_aux(iterator position, const value_type* first, const value_type* last, size_type n);
		void _insert_aux(iterator position, const_iterator first, const_iterator last, size_type n);

		iterator _reserve_elements_at_front(size_type n);
		iterator _reserve_elements_at_back(size_type n);
		void new_elements_at_front(size_type new_elements);
		void new_elements_at_back(size_type new_elements);

		void _reserve_map_at_front(size_type nodes_to_add = 1);
		void _reserve_map_at_back(size_type nodes_to_add = 1);
		void _reallocate_map(size_type nodes_to_add, bool add_at_front);
	};

	template<class T, class Alloc>
	void deque<T, Alloc>::_fill_initialize(const value_type& value)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_fill_insert(iterator position, size_type n, const value_type& x)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_pop_front_aux()
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_pop_back_aux()
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_front_aux()
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_front_aux(const value_type& t)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_back_aux()
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_back_aux(const value_type& t)
	{
	}

	template<class T, class Alloc>
	deque<T, Alloc>::iterator deque<T, Alloc>::_insert_aux(iterator position, const value_type& x)
	{
		return iterator();
	}

	template<class T, class Alloc>
	deque<T, Alloc>::iterator deque<T, Alloc>::_insert_aux(iterator position)
	{
		return iterator();
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_insert_aux(iterator position, size_type n, const value_type& x)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_insert_aux(iterator position, const value_type* first, const value_type* last, size_type n)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_insert_aux(iterator position, const_iterator first, const_iterator last, size_type n)
	{
	}

	template<class T, class Alloc>
	deque<T, Alloc>::iterator deque<T, Alloc>::_reserve_elements_at_front(size_type n)
	{
		return iterator();
	}

	template<class T, class Alloc>
	deque<T, Alloc>::iterator deque<T, Alloc>::_reserve_elements_at_back(size_type n)
	{
		return iterator();
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::new_elements_at_front(size_type new_elements)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::new_elements_at_back(size_type new_elements)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reserve_map_at_front(size_type nodes_to_add)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reserve_map_at_back(size_type nodes_to_add)
	{
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reallocate_map(size_type nodes_to_add, bool add_at_front)
	{
	}





}