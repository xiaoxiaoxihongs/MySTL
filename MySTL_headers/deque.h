#pragma once

#include "alloc.h"
#include "uninitialized.h"


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

	// �ú�������һ��ָ��T���͵�ָ�룬���Է���ֵ������Ҫ���������Ƶ�����T��αֵ
	// ���ڱ�д���벻ͬ���͵�����һ��ʹ�õķ��ʹ���
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
		// ��Ҫ�Ľڵ����Ԫ�ظ�������ÿ�������������ɵ�Ԫ�ظ��� +1�����������ô�ͻ������һ���ڵ�
		size_t num_nodes = num_elements / __deque_buf_size(sizeof(T)) + 1;

		// һ��mapҪ����Ľڵ�������8��������������ڵ���+2������ͷβ����
		_map_size = max((size_t)_initial_map_size, num_nodes + 2);
		_map = _allocate_map(_map_size);

		// nstart��nfinishָ��mapӵ�е�ȫ���ڵ�����������������������һ����ÿ���ڵ��Ӧһ��������
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
		
		//���������趨����ȷ��λ�ã���������һ���ڵ�ʱ��finish��cur��ָ����
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

		// ������ʱ��������쳣������ֹ������ȴû�й���
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
		// ��������
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
		// Ԫ��ָ���ָ��
		using map_pointer = pointer*;
		static size_t _buffer_size() { return __deque_buf_size(sizeof(_Tp)); }
		void _fill_initialize(const value_type& value);

	// ����д��������
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

	// ���캯�������д���ƶ�������������֪���Բ���
	public:
		explicit deque() : _Base(0) {};
		deque(const deque& x) : _Base(x.size()) { uninitialized_copy(x.cbegin(), x.cend(), _start); }
		deque(size_type n, const value_type& value) :_Base(n) { _fill_initialize(value); }
		explicit deque(size_type n) : _Base(n) { _fill_initialize(value_type()); }
		deque(const value_type* first, const value_type* last) :_Base(last - first) { uninitialized_copy(first, last, _start); }
		deque(const iterator first, const iterator last) : _Base(last - first) { uninitialized_copy(first, last); }
		
		// �ƶ�����������ʹ��һ��ͨ������������һ��������ʹ���ܹ����κ����͵�ֵ
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
					// �����ȴ���x�ĳ���ʱ����x��_startλ�ÿ�ʼ������Ȼ�������һ��Ԫ��֮��ĵ�����
					// �ٽ�����Ĳ���ɾ��
					erase(copy(x.cbegin(), x.cend(), _start));
				else
				{
					// ������С���Ǿ��Ƚ������ȵ����ݿ�����ȥ��Ȼ����ֱ�������������
					const_iterator mid = x.begin() + difference_type(len);
					copy(x.cbegin(), mid, _start);
					insert(_finish, mid, x.cend());
				}
			}
			return *this;
		}
		// �ƶ���ֵ���������֪���Բ���
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
			// ָ��β��Ԫ�ص���һ�������ƶ��ٴݻ�
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
			// ָ��ǰ��Ҫ���ݻٵ�Ԫ��
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
				//����ָ��β�����һ��
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
		// ɾ����
		iterator erase(iterator position)
		{
			iterator next = position;
			++next;
			// ����������֮ǰ��Ԫ�ر������֮���Ԫ���٣����ƶ������֮ǰ��Ԫ�ظ��ǣ�
			// Ȼ��ɾ����һ�����࣬��֮��Ȼ
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

		// ɾ����Χ
		iterator erase(iterator first, iterator last);

		// ���
		void clear();

	// resize
	public:
		void resize(size_type new_size, const value_type& x)
		{
			// ���С�ھͲ�����Ĳ���
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