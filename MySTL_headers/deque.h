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

		// 构造函数和拷贝构造函数
		_Deque_base(size_t num_elements) :
			_map(0), _map_size(0), _start(), _finish()
		{
			_initialize_map(num_elements);
		}
		_Deque_base(const _Deque_base& base) :
			_map(base._map), _map_size(base._map_size), _start(base._start), _finish(base._finish) {}
		// 移动构造函数
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

		// 分为两个空间配置，一个专门用于配置点，另一个用于配置map
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
		// 交换deque的数据
		void swap(deque& x)
		{
			std::swap(_map, x._map);
			std::swap(_map_size, x._map_size);
			std::swap(_start, x._start);
			std::swap(_finish, x._finish);
		}

	// push and pop
	public:
		// 用n个value填充
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
		// 在尾填充t
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
		// 在尾填充类型默认值
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
		// 在头填充t
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
		// 在头填充类型默认值
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
		// 尾弹出
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
		// 头弹出
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
		// 在position插入x
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
		// 在position插入类型默认值
		iterator insert(iterator position)
		{
			insert(position, value_type());
		}
		// 在position插入n个x
		void insert(iterator position, size_type n, const value_type& x)
		{
			_fill_insert(position, n, x);
		}

		// 在position位置插入一个元素的范围，例如arr[] = {1,2,3}, 那么first就是arr，last就是arr+3
		void insert(iterator position, const value_type* first, const value_type* last)
		{
			size_type n = last - first;
			if (position.cur == _start.cur)
			{
				iterator new_start = _reserve_elements_at_front(n);
				try
				{
					uninitialized_copy(first, last, new_start);
					_start = new_start;
				}
				catch (...)
				{
					_destroy_nodes(new_start.node, _start.node);
				}
			}
			else if (position.cur == _finish.cur)
			{
				iterator new_finish = _reserve_elements_at_back(n);
				try
				{
					uninitialized_copy(first, last, _finish);
					_finish = new_finish;
				}
				catch (...)
				{
					_destroy_nodes(_finish.node + 1, new_finish.node + 1);
				}
			}
			else
				_insert_aux(position, first, last, n);
		}
		// 在position位置插入一个元素的范围，接受迭代器
		void insert(iterator position, const_iterator first, const_iterator last)
		{
			size_type n = last - first;
			if (position.cur == _start.cur)
			{
				iterator new_start = _reserve_elements_at_front(n);
				try
				{
					uninitialized_copy(first, last, new_start);
					_start = new_start;
				}
				catch (...)
				{
					_destroy_nodes(new_start.node, _start.node);
				}
			}
			else if (position.cur == _finish.cur)
			{
				iterator new_finish = _reserve_elements_at_back(n);
				try
				{
					uninitialized_copy(first, last, _finish);
					_finish = new_finish;
				}
				catch (...)
				{
					_destroy_nodes(_finish.node + 1, new_finish.node + 1);
				}
			}
			else
				_insert_aux(position, first, last, n);

		}
		// 两段重复的代码为何不用模板来确定呢？还是说我只希望接收迭代器类型和指针类型，而不是其他例如同一个类
		// 哦，应该是这个理由，不要传些奇奇怪怪的东西进来
		
		// 可以试试newbing给的建议在note里面，到时候做做

	// erase
	public:
		// 删除position上的元素
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
		// 删除first到last范围内的元素
		iterator erase(iterator first, iterator last)
		{
			/*
			和删除位置一样的道理，如果删除的位置是全部，那就直接调用clear。
			计算元素个数和删除位置之前的元素，比较移动哪边的元素少，就覆盖哪边
			*/
			if (first == _start && last == _finish)
			{
				clear();
				return _finish;
			}
			else
			{
				difference_type n = last - first;
				difference_type elems_before = first - _start;
				if (elems_before < difference_type(this->size() - n) / 2)
				{
					copy_backward(_start, first, last);
					iterator new_start = _start + n;
					destroy(_start, new_start);
					_destroy_nodes(new_start.node, _start.node);
					_start = new_start;
				}
				else
				{
					copy(last, _finish, first);
					iterator new_finish = _finish - n;
					destroy(new_finish, _finish);
					_destroy_nodes(new_finish.node, _finish.node);
					_finish = new_finish;
				}
				return _start + elems_before;
			}
		}
		// 清空deque
		void clear()
		{
			// 遍历start.node + 1到finish.node之间的所有节点，摧毁元素并释放空间
			for (map_pointer tp_node= _start.node + 1; node < _finish.node; ++node)
			{
				destroy(*tp_node, tp_node + _buffer_size());
				_deallocate_node(*tp_node);
			}
			// 当start.node和finish.node不相等时，说明deque还有两个部分分别存放在
			// 这两个指针指向的节点，销毁中间部分，并释放finish.node节点
			if (_start.node != _finish.node)
			{
				destroy(_start.cur, _start.last);
				destroy(_finish.first, _finish.cur);
				_deallocate_node(_finish.first);
			}
			else
				destroy(_start.cur, _finish.cur);
			
			// 到最后会保留一个缓冲区，也就是deque的初始状态
			_finish = _start;
		}

	// resize
	public:
		// 重设deque大小
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
		// 重设deque大小，无参
		void resize(size_type new_size) { resize(new_size, value_type()); }
	protected:
		// 用value填充deque
		void _fill_initialize(const value_type& value);

		// 在position插入n个x，如果有必要的话，会调用 _reserve_map_at_front 或 _reserve_map_at_back 来预留 map 的空间。
		void _fill_insert(iterator position, size_type n, const value_type& x);

		// 删除 deque 的第一个元素，当且仅当_start.cur == _start.last - 1时调用
		void _pop_front_aux();

		// 删除 deque 的最后一个元素，当且仅当_finish.cur == _finish.first时调用
		void _pop_back_aux();

		// 在 deque 的前端插入一个默认构造的元素，当且仅当_start.cur == _start.last时调用
		void _push_front_aux();

		//  在 deque 的前端插入一个由 t 拷贝构造的元素，当且仅当_start.cur == _start.last时调用
		void _push_front_aux(const value_type& t);

		//  在 deque 的后端插入一个默认构造的元素，当且仅当_finish.cur == _finish.last - 1时调用
		void _push_back_aux();

		// 在 deque 的后端插入一个由 t 拷贝构造的元素，当且仅当_finish.cur == _finish.last - 1时调用
		void _push_back_aux(const value_type& t);

		// 在 position 位置插入一个由 x 拷贝构造的元素
		iterator _insert_aux(iterator position, const value_type& x);

		// 在 position 位置插入一个默认构造的元素
		iterator _insert_aux(iterator position);

		//  在 position 位置插入 n 个由 x 拷贝构造的元素
		void _insert_aux(iterator position, size_type n, const value_type& x);

		// 在 position 位置插入 [first, last) 范围内的 n 个元素
		void _insert_aux(iterator position, const value_type* first, const value_type* last, size_type n);

		// 在 position 位置插入 [first, last) 范围内的 n 个元素
		void _insert_aux(iterator position, const_iterator first, const_iterator last, size_type n);

		// 在deque的前端预留n个元素，返回指向第一个元素的迭代器
		iterator _reserve_elements_at_front(size_type n);

		// 在deque的后端预留n个元素，返回指向第一个元素的迭代器
		iterator _reserve_elements_at_back(size_type n);

		// 在deque的前端插入new_elements个新元素
		void new_elements_at_front(size_type new_elements);

		// 在deque的后端插入new_elements个新元素
		void new_elements_at_back(size_type new_elements);

		// 在deque的map前端预留nodes_to_add个节点
		void _reserve_map_at_front(size_type nodes_to_add = 1);

		// 在deque的map后端预留nodes_to_add个节点
		void _reserve_map_at_back(size_type nodes_to_add = 1);

		// 重分配map空间，以便在两端预留nodes_to_add个空间，add_at_front为true则预留在前，否则在后
		void _reallocate_map(size_type nodes_to_add, bool add_at_front);
	};

	template<class T, class Alloc>
	void deque<T, Alloc>::_fill_initialize(const value_type& value)
	{
		map_pointer cur;
		try
		{
			// 遍历deque中的每个节点node，*cur是指向一块连续内存的指针，*cur+buffer_size指向末尾
			/*为什么for中的判断条件是<, 如果是 <= 的话能去掉uninitialized_fill(_finish.first, _finish.cur, value)吗
			答：deque的内部结构是一个map，指向多个节点，每个节点指向一块连续内存，deque的迭代器由两部分组成
			一个是指向节点的指针，一个是指向元素的指针。deque的开始迭代器_start.node和_finish.node并不是指向第一个元素
			而finish.first等才是。
			所以for中判断条件是<,表示只遍历完整的节点，不包括最后一个节点。最后一个节点需要单独处理，有可能只有部分元素需要填充
			如果直接用<=的话，就会多填充，导致内存越界或覆盖已有元素等未定义行为。
			*/ 
			for (cur = _start.node; cur < _finish.node; ++cur)
				uninitialized_fill(*cur, *cur + _buffer_size, value);
			uninitialized_fill(_finish.first, _finish.cur, value);
		}
		catch (...)
		{
			destroy(_start, iterator(*cur, cur));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_fill_insert(iterator position, size_type n, const value_type& x)
	{
		if (position.cur == _start.cur)
		{
			iterator new_start = _reserve_elements_at_front(n);
			try
			{
				uninitialized_fill(new_start, _start, x);
				_start = new_start;
			}
			catch (...)
			{
				_destroy_nodes(new_start.node, _start.node);
			}
		}
		else if (position.cur == _finish.cur)
		{
			iterator new_finish = _reserve_elements_at_front(n);
			try
			{
				uninitialized_fill(_finish, new_finish, x);
				_finish = new_finish;
			}
			catch (...)
			{
				_destroy_nodes(_finish.node + 1, new_finish.node + 1);
			}
		}
		else
			_insert_aux(position, n, x);
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
		_reserve_map_at_front();
		*(_start.node - 1) = _allocate_node();
		try
		{
			_start._set_node(_start.node - 1);
			_start.cur = _start.last - 1;
			construct(_start.cur);
		}
		catch (...)
		{
			++_start;
			_deallocate_node(*(_start.node - 1));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_front_aux(const value_type& t)
	{
		value_type t_copy = t;
		_reserve_map_at_front();
		*(_start.node - 1) = _allocate_node();
		try
		{
			_start._set_node(_start.node - 1);
			_start.cur = _start.last - 1;
			construct(_start.cur, t_copy);
		}
		catch (...)
		{
			++_start;
			_deallocate_node(*(_start.node - 1));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_back_aux()
	{
		_reserve_map_at_back();
		*(_finish.node + 1) = _allocate_node();
		try
		{
			construct(_finish.cur);
			_finish._set_node(_finish.node + 1);
			_finish.cur = _finish.first;
		}
		catch (...)
		{
			_deallocate_node(*(_finish.node + 1));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_back_aux(const value_type& t)
	{
		value_type t_copy = t;
		_reserve_map_at_back();
		*(_finish.node + 1) = _allocate_node();
		try
		{
			construct(_finish.cur, t_copy);
			_finish._set_node(_finish.node + 1);
			_finish.cur = _finish.first;
		}
		catch (...)
		{
			_deallocate_node(*(_finish.node + 1));
		}
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
		// vacancies：空洞
		size_type vacancies = _start.cur - _start.first;
		if (n > vacancies)
			new_elements_at_front(n - vacancies);
		return _start - difference_type(n);
	}

	template<class T, class Alloc>
	deque<T, Alloc>::iterator deque<T, Alloc>::_reserve_elements_at_back(size_type n)
	{
		size_type vacancies = (_finish.last - _finish.cur) - 1;
		if (n > vacancies)
			new_elements_at_back(n - vacancies);
		return _finish + difference_type(n);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::new_elements_at_front(size_type new_elements)
	{
		size_type new_nodes = (new_elements + _buffer_size() - 1) / _buffer_size();
		_reserve_map_at_front(new_nodes);
		size_type i;
		try
		{
			for (i = 1; i < new_nodes; ++i)
				*(_finish.node - i) = _allocate_node();
		}
		catch (...)
		{
			for (size_type j = 1; j < i; ++j)
				_deallocate_node(*(_start.node - j));
			throw;
		}
		
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::new_elements_at_back(size_type new_elements)
	{
		// deque元素不是连续存储的，所有需要得到需要的节点数，并向上取整
		// 当new_elements是整数倍时，对结果没有影响；不是整数倍时，通过整数除法进行实现
		// 例如5/3向上取整，我们可以先让5+(3-1) = 7, 7/3 = 2,就是答案
		size_type new_nodes = (new_elements + _buffer_size() - 1) / _buffer_size();
		_reserve_map_at_back(new_nodes);
		size_type i;
		try
		{
			for (i = 1; i <= new_nodes; ++i)
				*(_finish.node + i) = _allocate_node();
		}
		catch (...) 
		{
			for (size_type j = 1; j < i; ++j)
				_deallocate_node(*(_finish.node + j));
			throw;
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reserve_map_at_front(size_type nodes_to_add)
	{
		// 如果当要添加的节点数大于map的起始位置和start.node之间的距离，那么就说明deque前端没有足够的空间
		if (nodes_to_add > size_type(_start.node - _map))
			_reallocate_map(nodes_to_add, true);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reserve_map_at_back(size_type nodes_to_add)
	{
		// 在后端添加需要保留一个空的缓冲区
		if (nodes_to_add + 1 > _map_size - (_finish.node - _map))
			_reallocate_map(nodes_to_add, false);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reallocate_map(size_type nodes_to_add, bool add_at_front)
	{
		// 疑问：重分配后的map指向包括了新的缓冲区吗？看起来不像啊
		// 计算原来节点数和新节点数
		size_type old_num_nodes = _finish.node - _start.node + 1;
		size_type new_num_nodes = old_num_nodes + nodes_to_add;

		map_pointer new_node_start;
		// 如果原map空间能够容纳足够的新节点数
		if (_map_size > 2 * new_num_nodes)
		{
			// 在map中找一个新的起始位置，把原来的节点复制过去
			/* 
			用于计算新的起始位置，在map中找到一个能够使新节点数居中对齐：(_map_size - new_num_nodes) / 2，
			让新节点映射居中的目的是平衡两端空间，提高插入和删除效率，减少映射指针失效的概率，防止空间浪费和不必要的内存分配
			并根据add_at_front的值，向前插入要偏移nodes_to_add个位置，想向后插入则不用。
			例如map有二十个位置，五个位置存储了节点，我们要在前端添加三个节点，此时new_num_nodes = 8
			那么new_node_start =_map + (20 - 8) / 2 + (true ? 3 : 0)也就是第10个位置
			*/
			new_node_start = _map + (_map_size - new_num_nodes) / 2 +
				(add_at_front ? nodes_to_add : 0);

			// 如果原开始位置大于新位置，那么将节点正向复制到新位置，如果小于那么就逆向复制
			// 用上面的例子来说，new_node_start = 10 > 0(_start.node),那么从后往前复制
			// 将原数组从15号开始复制到10号，只是挪动了原数据的位置
			if (new_node_start < _start.node)
				copy(_start.node, _finish.node + 1, new_node_start);
			else
				copy_backward(_start.node, _finish.node + 1, new_node_start + old_num_nodes);
		}
		else
		{
			// 例如map有十个位置，五个位置存储了节点，我们要在前端添加三个节点，此时new_num_nodes = 8
			// 此时先新建一个map，大小为_map_size + max(_map_size, nodes_to_add) + 2
			// （+2的目的在于为了避免映射指针指向映射边界，导致迭代器失效，如果有两个空闲位置那么映射指针在插入删除操作是不会越位）
			// ，然后计算位置，因为不涉及到赋值，那么直接将原map复制到新map
			// 的new_node_start位置，然后释放掉原map
			size_type new_map_size = _map_size + max(_map_size, nodes_to_add) + 2;
			map_pointer new_map = _allocate_map(new_map_size);
			new_node_start = new_map + (new_map_size - new_num_nodes) / 2 +
				(add_at_front ? nodes_to_add : 0);
			copy(_start.node, _finish.node + 1, new_node_start);
			_deallocate_map(_map, _map_size);

			_map = new_map;
			_map_size = new_map_size;
		}
		_start._set_node(new_node_start);
		_finish._set_node(new_node_start + old_num_nodes + 1);
	}
}