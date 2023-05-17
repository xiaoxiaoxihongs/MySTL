#pragma once
#include <iostream>
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
	inline size_t __deque_buf_size(size_t __size) {
		return __size < 512 ? size_t(512 / __size) : size_t(1);
	}
	// inline size_t __deque_buf_size(size_t size) { return size > 512 ? size_t(512 / size) : size_t(1); }

	template<class T, class Ref, class Ptr>
	struct _Deque_iterator
	{
		using iterator			= _Deque_iterator<T, T&, T*>;
		using const_iterator	= _Deque_iterator<T, const T&, const T*>;

		static size_t buffer_size() { return __deque_buf_size(sizeof(T)); }


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
		_Deque_iterator() :cur(0), first(0), last(0), node(0) {}
		_Deque_iterator(T* x, map_pointer y) :
			cur(x), first(*y), last(*y + buffer_size()), node(y) {}
		_Deque_iterator(const iterator& x):
			cur(x.cur), first(x.first), last(x.last), node(x.node) {}

		// �����������
		reference operator*()const { return *cur; }
		pointer operator->()const { return cur; }

		// ��ֵ�ļ��㷽���ǽ���������С��������������֮��Ļ���������
		// Ȼ�󽫵�������ƫ������ӵ�����ԵĻ�������,���� 01 2 3 4567 8 9 ab
		// ��������СΪ4����������ָ��ڶ���Ԫ�أ���һ��ָ�������Ԫ�أ�����3�ţ���һ����һ�ţ��м����2��
		// ��ô����֮��ľ���Ϊ4*(3-1-1) + (1-0) + (3-2) = 6;
		difference_type operator-(const self& x) const 
		{	
			return difference_type(buffer_size()) * (node - x.node - 1) +
				(cur - first) + (x.last - x.cur);
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
		const self operator++(int)
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
		const self operator--(int)
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
				_set_node(node + node_offset);
				cur = first + (offset - node_offset * difference_type(buffer_size()));
			}
			return *this;
		}

		// ����+=ʵ��
		// ����������һ��������Ա�����������޸ĵ�ǰ����״̬
		// ��temp���������Լ�����ı�ԭ��������ֵ
		self operator+(difference_type n) const
		{
			self temp = *this;
			return temp += n;
		}

		self& operator-=(difference_type n)
		{

			return *this += -n;
		}

		self operator-(difference_type n)
		{
			self temp = *this;
			return temp -= n;
		}

		reference operator[](difference_type n) const
		{
			return *(*this + n);
		}

		bool operator==(const self& x) const { return cur == x.cur; }
		bool operator!=(const self& x) const { return !(*this == x); }
		bool operator< (const self& x) const { return (node == x.node) ? (cur < x.cur) : (node < x.node); }
		bool operator<=(const self& x) const { return !(x < *this); }
		bool operator> (const self& x) const { return x < *this; }
		bool operator>=(const self& x) const { return !(*this < x); }

		void _set_node(map_pointer new_node)
		{
			node = new_node;
			first = *new_node;
			last = first + difference_type(buffer_size());
		}
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

		// ���캯���Ϳ������캯��
		_Deque_base() :_map(0), _map_size(0), _start(), _finish() {}
		_Deque_base(size_t num_elements) :
			_map(0), _map_size(0), _start(), _finish()
		{
			_initialize_map(num_elements);
		}
		_Deque_base(const _Deque_base& base) :
			_map(base._map), _map_size(base._map_size), _start(base._start), _finish(base._finish) {}
		// �ƶ����캯��
		template<typename U>
		_Deque_base(_Deque_base<U>&& base) noexcept :
			_map(base._map), _map_size(base._map_size), _start(base._start), _finish(base._finish) 
		{
			base._map = 0;
			base._map_size = 0;
			base._start = iterator();
			base._finish = iterator();
		}
		
		~_Deque_base();
	protected:
		T** _map;
		size_t _map_size;
		iterator _start;
		iterator _finish;

		enum {_initial_map_size = 8};

		// ��Ϊ�����ռ����ã�һ��ר���������õ㣬��һ����������map
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
	void _Deque_base<T, Alloc>::_initialize_map(size_t num_elements)
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
		_finish._set_node(nfinish - 1);
		_start.cur = _start.first;
		_finish.cur = _finish.first + num_elements % __deque_buf_size(sizeof(T));

	}

	template<class T, class Alloc>
	void _Deque_base<T, Alloc>::_create_nodes(T** _node_start, T** _node_finish)
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
		using value_type		= T;
		using pointer			= value_type*;
		using const_pointer		= const value_type*;
		using reference			= value_type&;
		using const_reference	= const value_type&;
		using size_type			= size_t;
		using difference_type	= ptrdiff_t;
		using iterator			= typename _Base::iterator;
		using const_iterator	= typename _Base::const_iterator;

	protected:
		// Ԫ��ָ���ָ��
		using map_pointer = pointer*;
		static size_t _buffer_size() { return __deque_buf_size(sizeof(T)); }

	// ����д��������
	public:
		iterator begin() { return this->_start; }
		const_iterator cbegin() const { return this->_start; }
		iterator end() { return this->_finish; }
		const_iterator cend() const { return this->_finish; }

		reference operator[](size_type n)
		{
			return this->_start[difference_type(n)];
		}
		const_reference operator[](size_type n) const
		{
			return this->_start[difference_type(n)];
		}

		// ��һ��Ԫ��
		reference front() { return *this->_start; }
		// ��һ��Ԫ��
		const_reference cfront() const { return *this->_start; }
		// ���һ��Ԫ��
		reference back() 
		{
			iterator temp = this->_finish;
			--temp;
			return *temp;
		}
		// ���һ��Ԫ��
		const_reference cback() const
		{
			iterator temp = this->_finish;
			--temp;
			return *temp;
		}

		// Ԫ�ظ���
		size_type size() const { return (this->_finish) - (this->_start); }
		// �������
		size_type max_size() const { return size_type(-1); }
		// �Ƿ�Ϊ��
		bool empty() const { return (this->_finish) == (this->_start); }

	// ���캯�������д���ƶ�������������֪���Բ���
	public:
		explicit deque() : _Base(0) {};
		deque(const deque& x) : _Base(x.size()) { m_uninitialized_copy(x.cbegin(), x.cend(), this->_start); }
		deque(size_type n, const value_type& value) :_Base(n) { _fill_initialize(value); }
		explicit deque(size_type n) : _Base(n) { _fill_initialize(value_type()); }
		deque(const value_type* first, const value_type* last) :_Base(last - first) { m_uninitialized_copy(first, last, this->_start); }
		deque(const iterator first, const iterator last) : _Base(last - first) { m_uninitialized_copy(first, last); }
		
		// �ƶ�����������ʹ��һ��ͨ������������һ��������ʹ���ܹ����κ����͵�ֵ
		template<typename U>
		deque(deque<U>&& x) :_Base(std::forward<deque<U>>(x)) {}

		~deque() { destroy(this->_start, this->_finish); }

	public:
		deque& operator=(const deque& x)
		{
			const size_type len = size();
			if (&x != this)
			{
				if (len >= x.size())
					// �����ȴ���x�ĳ���ʱ����x��_startλ�ÿ�ʼ������Ȼ�������һ��Ԫ��֮��ĵ�����
					// �ٽ�����Ĳ���ɾ��
					erase(m_copy(x.cbegin(), x.cend(), this->_start), this->_finish);
				else
				{
					// ������С���Ǿ��Ƚ������ȵ����ݿ�����ȥ��Ȼ����ֱ�������������
					const_iterator mid = x.begin() + difference_type(len);
					m_copy(x.cbegin(), mid, this->_start);
					insert(this->_finish, mid, x.cend());
				}
			}
			return *this;
		}
		// �ƶ���ֵ���������֪���Բ���
		deque& operator=(deque&& x) noexcept
		{
			swap(this->_map, x._map);
			swap(this->_map_size, x._map_size);
			swap(this->_start, x._start);
			swap(this->_finish, x._finish);
			return *this;
		}
		// ����deque������
		void swap(deque& x)
		{
			std::swap(this->_map, x._map);
			std::swap(this->_map_size, x._map_size);
			std::swap(this->_start, x._start);
			std::swap(this->_finish, x._finish);
		}

	// push and pop
	public:
		// ��n��value���ǣ����������ָ����������ڵ㣬��������ɾ������ڵ��ٸ���
		void fill_assign(size_type n, const T& value)
		{
			if (n > size())
			{
				m_fill(begin(), end(), value);
				insert(end(), n - size(), value);
			}
			else
			{
 				erase(begin() + n, end());
				m_fill(begin(), end(), value);
			}
		}
		// ��β���t
		void push_back(const value_type& t)
		{
			if (this->_finish.cur != this->_finish.last)
			{
				construct(this->_finish.cur, t);
				++this->_finish.cur;
			}
			else
				_push_back_aux(t);
		}
		// ��β�������Ĭ��ֵ
		void push_back()
		{
			if (this->_finish.cur != this->_finish.last)
			{
				construct(this->_finish.cur);
				++this->_finish.cur;
			}
			else
				_push_back_aux();
		}
		// ��ͷ���t
		void push_front(const value_type& t)
		{
			if (this->_start.cur != this->_start.first)
			{
				construct(this->_start.cur - 1, t);
				--this->_start.cur;
			}
			else
				_push_front_aux(t);
		}
		// ��ͷ�������Ĭ��ֵ
		void push_front()
		{
			if (this->_start.cur != this->_start.first)
			{
				construct(this->_start.cur - 1);
				--this->_start.cur;
			}
			else
				_push_front_aux();
		}
		// β����
		void pop_back()
		{
			// ָ��β��Ԫ�ص���һ�������ƶ��ٴݻ�
			if (this->_finish.cur != this->_finish.first)
			{
				--this->_finish.cur;
				destroy(this->_finish.cur);
			}
			else
				_pop_back_aux();
		}
		// ͷ����
		void pop_front()
		{
			// ָ��ǰ��Ҫ���ݻٵ�Ԫ��
			if (this->_start.cur != this->_start.last - 1)
			{
				destroy(this->_start.cur);
				++this->_start.cur;
			}
			else
				_pop_front_aux();
		}
		
	// insert
	public:
		// ��position����x
		iterator insert(iterator position, const value_type& x)
		{
			if (position.cur == this->_start.cur)
			{
				push_front(x);
				return this->_start;
			}
			else if (position.cur == this->_finish.cur)
			{
				push_back(x);
				//����ָ��β�����һ��
				iterator temp = this->_finish;
				--temp;
				return temp;
			}
			else
				return _insert_aux(position, x);
		}
		// ��position��������Ĭ��ֵ
		iterator insert(iterator position)
		{
			return insert(position, value_type());
		}
		// ��position����n��x
		void insert(iterator position, size_type n, const value_type& x)
		{
			_fill_insert(position, n, x);
		}

		// ��positionλ�ò���һ��Ԫ�صķ�Χ������arr[] = {1,2,3}, ��ôfirst����arr��last����arr+3
		void insert(iterator position, const value_type* first, const value_type* last)
		{
			size_type n = last - first;
			if (position.cur == this->_start.cur)
			{
				iterator new_start = _reserve_elements_at_front(n);
				try
				{
					m_uninitialized_copy(first, last, new_start);
					this->_start = new_start;
				}
				catch (...)
				{
					this->_destroy_nodes(new_start.node, this->_start.node);
				}
			}
			else if (position.cur == this->_finish.cur)
			{
				iterator new_finish = _reserve_elements_at_back(n);
				try
				{
					m_uninitialized_copy(first, last, this->_finish);
					this->_finish = new_finish;
				}
				catch (...)
				{
					this->_destroy_nodes(this->_finish.node + 1, new_finish.node + 1);
				}
			}
			else
				_insert_aux(position, first, last, n);
		}
		// ��positionλ�ò���һ��Ԫ�صķ�Χ�����ܵ�����
		void insert(iterator position, const_iterator first, const_iterator last)
		{
			size_type n = last - first;
			if (position.cur == this->_start.cur)
			{
				iterator new_start = _reserve_elements_at_front(n);
				try
				{
					m_uninitialized_copy(first, last, new_start);
					this->_start = new_start;
				}
				catch (...)
				{
					this->_destroy_nodes(new_start.node, this->_start.node);
				}
			}
			else if (position.cur == this->_finish.cur)
			{
				iterator new_finish = _reserve_elements_at_back(n);
				try
				{
					m_uninitialized_copy(first, last, this->_finish);
					this->_finish = new_finish;
				}
				catch (...)
				{
					this->_destroy_nodes(this->_finish.node + 1, new_finish.node + 1);
				}
			}
			else
				_insert_aux(position, first, last, n);

		}
		// �����ظ��Ĵ���Ϊ�β���ģ����ȷ���أ�����˵��ֻϣ�����յ��������ͺ�ָ�����ͣ���������������ͬһ����
		// Ŷ��Ӧ����������ɣ���Ҫ��Щ����ֵֹĶ�������
		
		// ��������newbing���Ľ�����note���棬��ʱ������

	// erase
	public:
		// ɾ��position�ϵ�Ԫ��
		iterator erase(iterator position)
		{
			iterator next = position;
			++next;
			// ����������֮ǰ��Ԫ�ر������֮���Ԫ���٣����ƶ������֮ǰ��Ԫ�ظ��ǣ�
			// Ȼ��ɾ����һ�����࣬��֮��Ȼ
			difference_type index = position - this->_start;
			if (size_type(index) < (this->size() >> 1))
			{
				m_copy_backward(this->_start, position, next);
				pop_front();
			}
			else
			{
				m_copy(next, this->_finish, position);
				pop_back();
			}
			return this->_start + index;
		}
		// ɾ��first��last��Χ�ڵ�Ԫ��
		iterator erase(iterator first, iterator last)
		{
			/*
			��ɾ��λ��һ���ĵ������ɾ����λ����ȫ�����Ǿ�ֱ�ӵ���clear��
			����Ԫ�ظ�����ɾ��λ��֮ǰ��Ԫ�أ��Ƚ��ƶ��ıߵ�Ԫ���٣��͸����ı�
			*/
			if (first == this->_start && last == this->_finish)
			{
				clear();
				return this->_finish;
			}
			else
			{
				difference_type n = last - first;
				difference_type elems_before = first - this->_start;
				if (elems_before < difference_type(this->size() - n) / 2)
				{
					m_copy_backward(this->_start, first, last);
					iterator new_start = this->_start + n;
					destroy(this->_start, new_start);
					this->_destroy_nodes(new_start.node, this->_start.node);
					this->_start = new_start;
				}
				else
				{
					m_copy(last, this->_finish, first);
					iterator new_finish = this->_finish - n;
					destroy(new_finish, this->_finish);
					this->_destroy_nodes(new_finish.node, this->_finish.node);
					this->_finish = new_finish;
				}
				return this->_start + elems_before;
			}
		}
		// ���deque
		void clear()
		{
			// ����start.node + 1��finish.node֮������нڵ㣬�ݻ�Ԫ�ز��ͷſռ�
			for (map_pointer tp_node= this->_start.node + 1; 
				tp_node < this->_finish.node; ++ tp_node)
			{
				destroy(*tp_node, *tp_node + _buffer_size());
				this->_deallocate_node(*tp_node);
			}
			// ��start.node��finish.node�����ʱ��˵��deque�����������ֱַ�����
			// ������ָ��ָ��Ľڵ㣬�����м䲿�֣����ͷ�finish.node�ڵ�
			if (this->_start.node != this->_finish.node)
			{
				destroy(this->_start.cur, this->_start.last);
				destroy(this->_finish.first, this->_finish.cur);
				this->_deallocate_node(this->_finish.first);
			}
			else
				destroy(this->_start.cur, this->_finish.cur);
			
			// �����ᱣ��һ����������Ҳ����deque�ĳ�ʼ״̬
			this->_finish = this->_start;
		}

	// resize
	public:
		// ����deque��С
		void resize(size_type new_size, const value_type& x)
		{
			// ���С�ھͲ�����Ĳ���
			const size_type len = size();
			if (new_size < len)
			{
				erase(this->_start + new_size, this->_finish);
			}
			else
				insert(this->_finish, new_size - len, x);
		}
		// ����deque��С���޲�
		void resize(size_type new_size) { resize(new_size, value_type()); }
	protected:
		// ��value���deque
		void _fill_initialize(const value_type& value);

		// ��position����n��x������б�Ҫ�Ļ�������� _reserve_map_at_front �� _reserve_map_at_back ��Ԥ�� map �Ŀռ䡣
		void _fill_insert(iterator position, size_type n, const value_type& x);

		// ��deque�ĵ�һ����ֻ��һ��Ԫ��ʱ���ã�������Ԫ�ز��ͷŵ��ڴ棬Ȼ��ָ����һ��
		void _pop_front_aux();

		// ��deque��β�����һ����Ϊ��ʱ�Żᱻ���ã������ٿտ飬Ȼ��������ǰһ���ڵ�����һ��Ԫ��
		void _pop_back_aux();

		// �� deque ��ǰ�˲���һ��Ĭ�Ϲ����Ԫ�أ����ҽ���_start.cur == _start.lastʱ����
		void _push_front_aux();

		//  �� deque ��ǰ�˲���һ���� t ���������Ԫ�أ����ҽ���_start.cur == _start.lastʱ����
		void _push_front_aux(const value_type& t);

		//  �� deque �ĺ�˲���һ��Ĭ�Ϲ����Ԫ�أ����ҽ���_finish.cur == _finish.last - 1ʱ����
		void _push_back_aux();

		// �� deque �ĺ�˲���һ���� t ���������Ԫ�أ����ҽ���_finish.cur == _finish.last - 1ʱ����
		void _push_back_aux(const value_type& t);

		// �� position λ�ò���һ���� x ���������Ԫ��
		iterator _insert_aux(iterator position, const value_type& x);

		// �� position λ�ò���һ��Ĭ�Ϲ����Ԫ��
		iterator _insert_aux(iterator position);

		//  �� position λ�ò��� n ���� x ���������Ԫ��
		void _insert_aux(iterator position, size_type n, const value_type& x);

		// �� position λ�ò��� [first, last) ��Χ�ڵ� n ��Ԫ��
		void _insert_aux(iterator position, const value_type* first, const value_type* last, size_type n);

		// �� position λ�ò��� [first, last) ��Χ�ڵ� n ��Ԫ��
		void _insert_aux(iterator position, const_iterator first, const_iterator last, size_type n);

		// �� deque ��ǰ��Ԥ��n��Ԫ�أ�����ָ���һ��Ԫ�صĵ�����
		iterator _reserve_elements_at_front(size_type n);

		// �� deque �ĺ��Ԥ��n��Ԫ�أ�����ָ���һ��Ԫ�صĵ�����
		iterator _reserve_elements_at_back(size_type n);

		// �� deque ��ǰ�˲���new_elements����Ԫ��
		void new_elements_at_front(size_type new_elements);

		// �� deque �ĺ�˲���new_elements����Ԫ��
		void new_elements_at_back(size_type new_elements);

		// �� deque ��mapǰ��Ԥ��nodes_to_add���ڵ�
		void _reserve_map_at_front(size_type nodes_to_add = 1);

		// �� deque ��map���Ԥ��nodes_to_add���ڵ�
		void _reserve_map_at_back(size_type nodes_to_add = 1);

		// �ط���map�ռ䣬�Ա�������Ԥ��nodes_to_add���ռ䣬add_at_frontΪtrue��Ԥ����ǰ�������ں�
		void _reallocate_map(size_type nodes_to_add, bool add_at_front);
	};

	template<class T, class Alloc>
	void deque<T, Alloc>::_fill_initialize(const value_type& value)
	{
		map_pointer cur;
		try
		{
			// ����deque�е�ÿ���ڵ�node��*cur��ָ��һ�������ڴ��ָ�룬*cur+buffer_sizeָ��ĩβ
			/*Ϊʲôfor�е��ж�������<, ����� <= �Ļ���ȥ��uninitialized_fill(_finish.first, _finish.cur, value)��
			��deque���ڲ��ṹ��һ��map��ָ�����ڵ㣬ÿ���ڵ�ָ��һ�������ڴ棬deque�ĵ����������������
			һ����ָ��ڵ��ָ�룬һ����ָ��Ԫ�ص�ָ�롣deque�Ŀ�ʼ������_start.node��_finish.node������ָ���һ��Ԫ��
			��finish.first�Ȳ��ǡ�
			����for���ж�������<,��ʾֻ���������Ľڵ㣬���������һ���ڵ㡣���һ���ڵ���Ҫ���������п���ֻ�в���Ԫ����Ҫ���
			���ֱ����<=�Ļ����ͻ����䣬�����ڴ�Խ��򸲸�����Ԫ�ص�δ������Ϊ��
			*/ 
			for (cur = this->_start.node; cur < this->_finish.node; ++cur)
				m_uninitialized_fill(*cur, *cur + _buffer_size, value);
			m_uninitialized_fill(this->_finish.first, this->_finish.cur, value);
		}
		catch (...)
		{
			destroy(this->_start, iterator(*cur, cur));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_fill_insert(iterator position, size_type n, const value_type& x)
	{
		if (position.cur == this->_start.cur)
		{
			iterator new_start = _reserve_elements_at_front(n);
			try
			{
				m_uninitialized_fill(new_start, this->_start, x);
				this->_start = new_start;
			}
			catch (...)
			{
				this->_destroy_nodes(new_start.node, this->_start.node);
			}
		}
		else if (position.cur == this->_finish.cur)
		{
			iterator new_finish = _reserve_elements_at_back(n);
			try
			{
				m_uninitialized_fill(this->_finish, new_finish, x);
				this->_finish = new_finish;
			}
			catch (...)
			{
				this->_destroy_nodes(this->_finish.node + 1, new_finish.node + 1);
			}
		}
		else
			_insert_aux(position, n, x);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_pop_front_aux()
	{
		destroy(this->_start.cur);
		this->_deallocate_node(this->_start.first);
		this->_start._set_node(this->_start.node + 1);
		this->_start.cur = this->_start.first;
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_pop_back_aux()
	{
		this->_deallocate_node(this->_finish.first);
		this->_finish._set_node(this->_finish.node - 1);
		this->_finish.cur = this->_finish.last - 1;
		destroy(this->_finish.cur);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_front_aux()
	{
		_reserve_map_at_front();
		*(this->_start.node - 1) = this->_allocate_node();
		try
		{
			this->_start._set_node(this->_start.node - 1);
			this->_start.cur = this->_start.last - 1;
			construct(this->_start.cur);
		}
		catch (...)
		{
			++this->_start;
			_deallocate_node(*(this->_start.node - 1));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_front_aux(const value_type& t)
	{
		value_type t_copy = t;
		_reserve_map_at_front();
		*(this->_start.node - 1) = this->_allocate_node();
		try
		{
			this->_start._set_node(this->_start.node - 1);
			this->_start.cur = this->_start.last - 1;
			construct(this->_start.cur, t_copy);
		}
		catch (...)
		{
			++this->_start;
			this->_deallocate_node(*(this->_start.node - 1));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_back_aux()
	{
		_reserve_map_at_back();
		*(this->_finish.node + 1) = this->_allocate_node();
		try
		{
			construct(this->_finish.cur);
			this->_finish._set_node(this->_finish.node + 1);
			this->_finish.cur = this->_finish.first;
		}
		catch (...)
		{
			this->_deallocate_node(*(this->_finish.node + 1));
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_push_back_aux(const value_type& t)
	{
		value_type t_copy = t;
		_reserve_map_at_back();
		*(this->_finish.node + 1) = this->_allocate_node();
		try
		{
			construct(this->_finish.cur, t_copy);
			this->_finish._set_node(this->_finish.node + 1);
			this->_finish.cur = this->_finish.first;
		}
		catch (...)
		{
			this->_deallocate_node(*(this->_finish.node + 1));
		}
	}

	template<class T, class Alloc>
	typename deque<T, Alloc>::iterator deque<T, Alloc>::_insert_aux(iterator position, const value_type& x)
	{
		difference_type index = position - this->_start;
		value_type x_copy = x;
		// ����֮���ٵĵط�����
		if (size_type(index) < this->size() / 2)
		{
			// ����ͷ�ڵ㴦���Ʋ���һ��ͷԪ�أ�Ȼ��ȡ��ԭͷ�ڵ�֮��һλ��position�ϵĵ�������
			// ����������Χ�ڵĲ������ǵ�ԭͷ�ڵ���
			push_front(front());
			iterator front1 = this->_start;
			++front1;
			iterator front2 = front1;
			++front2;
			position = this->_start + index;
			iterator pos = position;
			++pos;
			m_copy(front2, pos, front1);
		}
		else
		{
			// ��β�ڵ㴦����һ��βԪ�أ�Ȼ���ԭβԪ��ǰһλ��position�ϵĵ�����
			// ����������Χ�ڵĲ������ǵ�ԭͷ�ڵ���
			push_back(back());
			iterator back1 = this->_finish;
			--back1;
			iterator back2 = this->_finish;
			--back2;
			position = this->_start + index;
			m_copy_backward(position, back2, back1);
		}
		*position = x_copy;
		return position;
	}

	template<class T, class Alloc>
	typename deque<T, Alloc>::iterator deque<T, Alloc>::_insert_aux(iterator position)
	{
		// Ϊɶ����_insert_aux(position, T());ֱ��ί�г�ȥ�أ�

		const difference_type index = position - this->_start;
		// ����֮���ٵĵط�����
		if (index < size() / 2)
		{
			// ����ͷ�ڵ㴦���Ʋ���һ��ͷԪ�أ�Ȼ��ȡ��ԭͷ�ڵ�֮��һλ��position�ϵĵ�������
			// ����������Χ�ڵĲ������ǵ�ԭͷ�ڵ���
			push_front(front());
			iterator front1 = this->_start;
			++front1;
			iterator front2 = front1;
			++front2;
			position = this->_start + index;
			iterator pos = position;
			++pos;
			m_copy(front2, pos, front1);
		}
		else
		{
			// ��β�ڵ㴦����һ��βԪ�أ�Ȼ���ԭβԪ��ǰһλ��position�ϵĵ�����
			// ����������Χ�ڵĲ������ǵ�ԭͷ�ڵ���
			push_back(back());
			iterator back1 = this->_finish;
			--back1;
			iterator back2 = this->_finish;
			--back2;
			position = this->_start + index;
			m_copy_backward(position, back2, back1);
		}
		*position = value_type();
		return position;
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_insert_aux(iterator position, size_type n, const value_type& x)
	{
		// ����λ��֮ǰ��Ԫ��
		const difference_type elements_before = position - this->_start;
		// deque����
		size_type len = this->size();
		value_type x_copy = x;
		// ��
		if (elements_before < difference_type(len / 2))
		{
			// ��¼Ԥ��n��λ�ú�start�Ŀ�ʼ
			iterator new_start = _reserve_elements_at_front(n);
			iterator old_start = this->_start;
			// _reserve_elements_at_front(n)֮����������ܻ�ʧЧ��ָ��ͬλ�ã�������Ҫ����һ��
			position = this->_start + elements_before;
			try
			{
				// ������Ԫ��֮ǰ��Ԫ�ش���Ҫ���������ʱ
				if (elements_before >= difference_type(n))
				{
					// ԭͷ�ڵ�+n
					iterator start_n = this->_start + difference_type(n);
					// ��ԭͷ�ڵ㵽n-1(����ҿ�ԭ��)����Ԫ�ؿ�������ͷ�ڵ��λ�ã���Ϊuninitialized_copyֻ����δ��ʼ���Ŀռ����
					// ���Բ���һ����λ�Ľ���uninitialized_copy(_start, _start_n + 1, new_start);
					m_uninitialized_copy(this->_start, start_n, new_start);
					this->_start = new_start;
					// ��_start_n��position��Ԫ�ؿ�����ԭͷ�ڵ�֮����ʵ����һ���ڵ�
					m_copy(start_n, position, old_start);
					// ��󽫿ճ�����λ�����
					m_fill(position - difference_type(n), position, x_copy);
					/*
					��3��λ���2��1
					1 2 3 4 5 6 7 8 9 -> _ _ 1 2 3 4 5 6 7 8 9 -> 1 2 1 2 3 4 5 6 7 8 9 
					-> 1 2 3 2 3 4 5 6 7 8 9 -> 1 2 3 1 1 4 5 6 7 8 9
					*/
				}
				else
				{
					// ����δ��ʼ���Ŀռ��Ͽ���[_start, position)
					iterator mid = m_uninitialized_copy(this->_start, position, new_start);
					try
					{
						// Ȼ��ʣ��δ��ʼ���Ŀռ����value
						m_uninitialized_fill(mid, this->_start, x_copy);
					}
					catch (...)
					{
						destroy(new_start, mid);
					}
					this->_start = new_start;
					// �ٽ�������������value
					m_fill(old_start, position, x_copy);
					/*
					��3��λ���4��1
					1 2 3 4 5 6 7 8 9 -> _ _ _ _ 1 2 3 4 5 6 7 8 9 -> 1 2 3 _ 1 2 3 4 5 6 7 8 9 ->
					1 2 3 1 1 2 3 4 5 6 7 8 9 -> 1 2 3 1 1 1 1 4 5 6 7 8 9
					*/
				}
			}
			catch (...)
			{
				this->_destroy_nodes(new_start.node, this->_start.node);
			}
		}
		else
		{
			// ͬ����˼·�����ں���Ԥ��Ԫ�أ�Ȼ����������λ��֮���Ԫ�ظ���
			iterator new_finish = _reserve_elements_at_back(n);
			iterator old_finish = this->_finish;
			const difference_type elements_after = difference_type(len) - elements_before;
			// ����position
			position = this->_finish - elements_before;
			try
			{
				// �������Ԫ�ش���Ҫ�����Ԫ��ʱ
				if (elements_after > difference_type(n))
				{
					// ��������ڵ�(��С����n������������δ��ʼ���Ŀռ���)
					iterator finish_n = this->_finish - difference_type(n);
					m_uninitialized_copy(finish_n, this->_finish, this->_finish);
					this->_finish = new_finish;
					// �Ӻ���ǰ��Ҳ�ǲ���һ����λ�Ľ���
					m_copy_backward(position, finish_n, old_finish);
					m_fill(position, position + difference_type(n), x_copy);
					/*
					��6��λ���2��1
					1 2 3 4 5 6 7 8 9 -> 1 2 3 4 5 6 7 8 9 _ _ -> 1 2 3 4 5 6 7 8 9 8 9 
					-> 1 2 3 4 5 6 7 8 7 8 9 -> 1 2 3 4 5 6 1 1 7 8 9
					*/
				}
				else
				{
					// �������Ԫ��С�ڵ���Ҫ�����Ԫ��ʱ���ȼ������㵽n��λ�ã��ᳬ��_finish
					iterator mid = position + difference_type(n);
					// ��δ��ʼ���ĵط����position����Ĵ�С��ֱ�ӳ�ʼ����value
					m_uninitialized_fill(this->_finish, mid, x_copy);
					try
					{
						// Ȼ���ٽ�position��_finish��Ԫ�س�ʼ�������
						m_uninitialized_copy(position, this->_finish, mid);
					}
					catch (...)
					{
						destroy(this->_finish, mid);
					}
					this->_finish = new_finish;
					// �ٽ�ԭԪ�ظ���
					m_fill(position, old_finish, x_copy);
					/*
					��6��λ���4��1
					1 2 3 4 5 6 7 8 9 -> 1 2 3 4 5 6 7 8 9 _ _ _ _-> 1 2 3 4 5 6 7 8 9 1 _ _ _
					-> 1 2 3 4 5 6 7 8 9 1 7 8 9 -> 1 2 3 4 5 6 1 1 1 1 7 8 9 
					*/
				}
			}
			catch (...)
			{
				this->_destroy_nodes(this->_finish.node + 1, new_finish.node + 1);
			}
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_insert_aux(iterator position, const value_type* first, const value_type* last, size_type n)
	{
		const difference_type elements_before = position - this->_start;
		size_type len = this->size();
		if (elements_before < difference_type(len / 2))
		{
			iterator new_start = _reserve_elements_at_front(n);
			iterator old_start = this->_start;
			position = this->_start + elements_before;
			try
			{
				if (elements_before >= difference_type(n))
				{
					iterator _start_n = this->_start + difference_type(n);
					m_uninitialized_copy(this->_start, _start_n, new_start);
					this->_start = new_start;
					m_copy(_start_n, position, old_start);
					m_copy(first, last, position - difference_type(n));
				}
				else
				{
					const value_type* mid = first + difference_type(n) - elements_before;
					iterator _mid = m_uninitialized_copy(this->_start, position, new_start);
					try
					{
						m_uninitialized_copy(first, mid, _mid);
					}
					catch (...)
					{
						destroy(this->_start, _mid);
					}
					
					this->_start = new_start;
					m_copy(mid, last, old_start);
				}
			}
			catch (...)
			{
				this->_destroy_nodes(new_start.node, this->_start.node);
			}
		}
		else
		{
			iterator new_finish = _reserve_elements_at_back(n);
			iterator old_finish = this->_finish;
			const difference_type elements_after = difference_type(len) - elements_before;
			position = this->_finish - elements_after;
			try
			{
				if (elements_after > difference_type(n))
				{
					iterator finish_n = this->_finish - difference_type(n);
					m_uninitialized_copy(finish_n, this->_finish, this->_finish);
					this->_finish = new_finish;
					m_copy_backward(position, finish_n, old_finish);
					m_copy(first, last, position);
				}
				else
				{
					const value_type* mid = first + elements_after;
					iterator _mid = m_uninitialized_copy(mid, last, this->_finish);
					try
					{
						m_uninitialized_copy(position, this->_finish, _mid);
					}
					catch (...)
					{
						destroy(this->_finish, _mid);
					}
					this->_finish = new_finish;
					m_copy(first, mid, position);
				}
			}
			catch (...)
			{
				this->_destroy_nodes(this->_finish.node + 1, new_finish.node + 1);
			}
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_insert_aux(iterator position, const_iterator first, const_iterator last, size_type n)
	{
		const difference_type elements_before = position - this->_start;
		size_type len = this->size();
		if (elements_before < difference_type(len / 2))
		{
			iterator new_start = _reserve_elements_at_front(n);
			iterator old_start = this->_start;
			position = this->_start + elements_before;
			try
			{
				if (elements_before >= difference_type(n))
				{
					iterator _start_n = this->_start + difference_type(n);
					m_uninitialized_copy(this->_start, _start_n, new_start);
					this->_start = new_start;
					m_copy(_start_n, position, old_start);
					m_copy(first, last, position - difference_type(n));
				}
				else
				{
					const_iterator mid = first + difference_type(n) - elements_before;
					iterator _mid = m_uninitialized_copy(this->_start, position, new_start);
					try
					{
						m_uninitialized_copy(first, mid, _mid);
					}
					catch (...)
					{
						destroy(this->_start, _mid);
					}

					this->_start = new_start;
					m_copy(mid, last, old_start);
				}
			}
			catch (...)
			{
				this->_destroy_nodes(new_start.node, this->_start.node);
			}
		}
		else
		{
			iterator new_finish = _reserve_elements_at_back(n);
			iterator old_finish = this->_finish;
			const difference_type elements_after = difference_type(len) - elements_before;
			position = this->_finish - elements_after;
			try
			{
				if (elements_after > difference_type(n))
				{
					iterator finish_n = this->_finish - difference_type(n);
					m_uninitialized_copy(finish_n, this->_finish, this->_finish);
					this->_finish = new_finish;
					m_copy_backward(position, finish_n, old_finish);
					m_copy(first, last, position);
				}
				else
				{
					const_iterator mid = first + elements_after;
					iterator _mid = m_uninitialized_copy(mid, last, this->_finish);
					try
					{
						m_uninitialized_copy(position, this->_finish, _mid);
					}
					catch (...)
					{
						destroy(this->_finish, _mid);
					}
					this->_finish = new_finish;
					m_copy(first, mid, position);
				}
			}
			catch (...)
			{
				this->_destroy_nodes(this->_finish.node + 1, new_finish.node + 1);
			}
		}
	}

	template<class T, class Alloc>
	typename deque<T, Alloc>::iterator deque<T, Alloc>::_reserve_elements_at_front(size_type n)
	{
		// vacancies���ն�
		size_type vacancies = this->_start.cur - this->_start.first;
		if (n > vacancies)
			new_elements_at_front(n - vacancies);
		return this->_start - difference_type(n);
	}

	template<class T, class Alloc>
	typename deque<T, Alloc>::iterator deque<T, Alloc>::_reserve_elements_at_back(size_type n)
	{
		size_type vacancies = (this->_finish.last - this->_finish.cur) - 1;
		if (n > vacancies)
			new_elements_at_back(n - vacancies);
		return this->_finish + difference_type(n);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::new_elements_at_front(size_type new_elements)
	{
		size_type new_nodes = (new_elements + _buffer_size() - 1) / _buffer_size();
		_reserve_map_at_front(new_nodes);
		size_type i;
		try
		{
			for (i = 1; i <= new_nodes; ++i)
				*(this->_finish.node - i) = this->_allocate_node();
		}
		catch (...)
		{
			for (size_type j = 1; j < i; ++j)
				this->_deallocate_node(*(this->_start.node - j));
			throw;
		}
		
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::new_elements_at_back(size_type new_elements)
	{
		// dequeԪ�ز��������洢�ģ�������Ҫ�õ���Ҫ�Ľڵ�����������ȡ��
		// ��new_elements��������ʱ���Խ��û��Ӱ�죻����������ʱ��ͨ��������������ʵ��
		// ����5/3����ȡ�������ǿ�������5+(3-1) = 7, 7/3 = 2,���Ǵ�
		size_type new_nodes = (new_elements + _buffer_size() - 1) / _buffer_size();
		_reserve_map_at_back(new_nodes);
		size_type i;
		try
		{
			for (i = 1; i <= new_nodes; ++i)
				*(this->_finish.node + i) = this->_allocate_node();
		}
		catch (...) 
		{
			for (size_type j = 1; j < i; ++j)
				this->_deallocate_node(*(this->_finish.node + j));
			throw;
		}
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reserve_map_at_front(size_type nodes_to_add)
	{
		// �����Ҫ��ӵĽڵ�������map����ʼλ�ú�start.node֮��ľ��룬��ô��˵��dequeǰ��û���㹻�Ŀռ�
		if (nodes_to_add > size_type(this->_start.node - this->_map))
			_reallocate_map(nodes_to_add, true);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reserve_map_at_back(size_type nodes_to_add)
	{
		// �ں�������Ҫ����һ���յĻ�����
		if (nodes_to_add + 1 > this->_map_size - (this->_finish.node - this->_map))
			_reallocate_map(nodes_to_add, false);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reallocate_map(size_type nodes_to_add, bool add_at_front)
	{
		// ���ʣ��ط�����mapָ��������µĻ������𣿿���������
		// ����ԭ���ڵ������½ڵ���
		size_type old_num_nodes = this->_finish.node - this->_start.node + 1;
		size_type new_num_nodes = old_num_nodes + nodes_to_add;

		map_pointer new_node_start;
		// ���ԭmap�ռ��ܹ������㹻���½ڵ���
		if (this->_map_size > 2 * new_num_nodes)
		{
			// ��map����һ���µ���ʼλ�ã���ԭ���Ľڵ㸴�ƹ�ȥ
			/* 
			���ڼ����µ���ʼλ�ã���map���ҵ�һ���ܹ�ʹ�½ڵ������ж��룺(_map_size - new_num_nodes) / 2��
			���½ڵ�ӳ����е�Ŀ����ƽ�����˿ռ䣬��߲����ɾ��Ч�ʣ�����ӳ��ָ��ʧЧ�ĸ��ʣ���ֹ�ռ��˷ѺͲ���Ҫ���ڴ����
			������add_at_front��ֵ����ǰ����Ҫƫ��nodes_to_add��λ�ã������������á�
			����map�ж�ʮ��λ�ã����λ�ô洢�˽ڵ㣬����Ҫ��ǰ����������ڵ㣬��ʱnew_num_nodes = 8
			��ônew_node_start =_map + (20 - 8) / 2 + (true ? 3 : 0)Ҳ���ǵ�10��λ��
			*/
			new_node_start = this->_map + (this->_map_size - new_num_nodes) / 2 +
				(add_at_front ? nodes_to_add : 0);

			// ���ԭ��ʼλ�ô�����λ�ã���ô���ڵ������Ƶ���λ�ã����С����ô��������
			// �������������˵��new_node_start = 10 > 0(_start.node),��ô�Ӻ���ǰ����
			// ��ԭ�����15�ſ�ʼ���Ƶ�10�ţ�ֻ��Ų����ԭ���ݵ�λ��
			if (new_node_start < this->_start.node)
				m_copy(this->_start.node, this->_finish.node + 1, new_node_start);
			else
				m_copy_backward(this->_start.node, this->_finish.node + 1, new_node_start + old_num_nodes);
		}
		else
		{
			// ����map��ʮ��λ�ã����λ�ô洢�˽ڵ㣬����Ҫ��ǰ����������ڵ㣬��ʱnew_num_nodes = 8
			// ��ʱ���½�һ��map����СΪ_map_size + max(_map_size, nodes_to_add) + 2
			// ��+2��Ŀ������Ϊ�˱���ӳ��ָ��ָ��ӳ��߽磬���µ�����ʧЧ���������������λ����ôӳ��ָ���ڲ���ɾ�������ǲ���Խλ��
			// ��Ȼ�����λ�ã���Ϊ���漰����ֵ����ôֱ�ӽ�ԭmap���Ƶ���map
			// ��new_node_startλ�ã�Ȼ���ͷŵ�ԭmap
			size_type new_map_size = this->_map_size + max(this->_map_size, nodes_to_add) + 2;
			map_pointer new_map = this->_allocate_map(new_map_size);
			new_node_start = new_map + (new_map_size - new_num_nodes) / 2 +
				(add_at_front ? nodes_to_add : 0);
			m_copy(this->_start.node, this->_finish.node + 1, new_node_start);
			this->_deallocate_map(this->_map, this->_map_size);

			this->_map = new_map;
			this->_map_size = new_map_size;
		}
		this->_start._set_node(new_node_start);
		this->_finish._set_node(new_node_start + old_num_nodes + 1);
	}
}