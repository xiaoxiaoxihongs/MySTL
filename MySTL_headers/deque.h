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

		// ���캯���Ϳ������캯��
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
		// ����deque������
		void swap(deque& x)
		{
			std::swap(_map, x._map);
			std::swap(_map_size, x._map_size);
			std::swap(_start, x._start);
			std::swap(_finish, x._finish);
		}

	// push and pop
	public:
		// ��n��value���
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
		// ��β���t
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
		// ��β�������Ĭ��ֵ
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
		// ��ͷ���t
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
		// ��ͷ�������Ĭ��ֵ
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
		// β����
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
		// ͷ����
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
		// ��position����x
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
		// ��position��������Ĭ��ֵ
		iterator insert(iterator position)
		{
			insert(position, value_type());
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
		// ��positionλ�ò���һ��Ԫ�صķ�Χ�����ܵ�����
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
		// ɾ��first��last��Χ�ڵ�Ԫ��
		iterator erase(iterator first, iterator last)
		{
			/*
			��ɾ��λ��һ���ĵ������ɾ����λ����ȫ�����Ǿ�ֱ�ӵ���clear��
			����Ԫ�ظ�����ɾ��λ��֮ǰ��Ԫ�أ��Ƚ��ƶ��ıߵ�Ԫ���٣��͸����ı�
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
		// ���deque
		void clear()
		{
			// ����start.node + 1��finish.node֮������нڵ㣬�ݻ�Ԫ�ز��ͷſռ�
			for (map_pointer tp_node= _start.node + 1; node < _finish.node; ++node)
			{
				destroy(*tp_node, tp_node + _buffer_size());
				_deallocate_node(*tp_node);
			}
			// ��start.node��finish.node�����ʱ��˵��deque�����������ֱַ�����
			// ������ָ��ָ��Ľڵ㣬�����м䲿�֣����ͷ�finish.node�ڵ�
			if (_start.node != _finish.node)
			{
				destroy(_start.cur, _start.last);
				destroy(_finish.first, _finish.cur);
				_deallocate_node(_finish.first);
			}
			else
				destroy(_start.cur, _finish.cur);
			
			// �����ᱣ��һ����������Ҳ����deque�ĳ�ʼ״̬
			_finish = _start;
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
				erase(_start + new_size, _finish);
			}
			else
				insert(_finish, new_size - len, x);
		}
		// ����deque��С���޲�
		void resize(size_type new_size) { resize(new_size, value_type()); }
	protected:
		// ��value���deque
		void _fill_initialize(const value_type& value);

		// ��position����n��x������б�Ҫ�Ļ�������� _reserve_map_at_front �� _reserve_map_at_back ��Ԥ�� map �Ŀռ䡣
		void _fill_insert(iterator position, size_type n, const value_type& x);

		// ɾ�� deque �ĵ�һ��Ԫ�أ����ҽ���_start.cur == _start.last - 1ʱ����
		void _pop_front_aux();

		// ɾ�� deque �����һ��Ԫ�أ����ҽ���_finish.cur == _finish.firstʱ����
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

		// ��deque��ǰ��Ԥ��n��Ԫ�أ�����ָ���һ��Ԫ�صĵ�����
		iterator _reserve_elements_at_front(size_type n);

		// ��deque�ĺ��Ԥ��n��Ԫ�أ�����ָ���һ��Ԫ�صĵ�����
		iterator _reserve_elements_at_back(size_type n);

		// ��deque��ǰ�˲���new_elements����Ԫ��
		void new_elements_at_front(size_type new_elements);

		// ��deque�ĺ�˲���new_elements����Ԫ��
		void new_elements_at_back(size_type new_elements);

		// ��deque��mapǰ��Ԥ��nodes_to_add���ڵ�
		void _reserve_map_at_front(size_type nodes_to_add = 1);

		// ��deque��map���Ԥ��nodes_to_add���ڵ�
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
		// vacancies���ն�
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
		// dequeԪ�ز��������洢�ģ�������Ҫ�õ���Ҫ�Ľڵ�����������ȡ��
		// ��new_elements��������ʱ���Խ��û��Ӱ�죻����������ʱ��ͨ��������������ʵ��
		// ����5/3����ȡ�������ǿ�������5+(3-1) = 7, 7/3 = 2,���Ǵ�
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
		// �����Ҫ��ӵĽڵ�������map����ʼλ�ú�start.node֮��ľ��룬��ô��˵��dequeǰ��û���㹻�Ŀռ�
		if (nodes_to_add > size_type(_start.node - _map))
			_reallocate_map(nodes_to_add, true);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reserve_map_at_back(size_type nodes_to_add)
	{
		// �ں�������Ҫ����һ���յĻ�����
		if (nodes_to_add + 1 > _map_size - (_finish.node - _map))
			_reallocate_map(nodes_to_add, false);
	}

	template<class T, class Alloc>
	void deque<T, Alloc>::_reallocate_map(size_type nodes_to_add, bool add_at_front)
	{
		// ���ʣ��ط�����mapָ��������µĻ������𣿿���������
		// ����ԭ���ڵ������½ڵ���
		size_type old_num_nodes = _finish.node - _start.node + 1;
		size_type new_num_nodes = old_num_nodes + nodes_to_add;

		map_pointer new_node_start;
		// ���ԭmap�ռ��ܹ������㹻���½ڵ���
		if (_map_size > 2 * new_num_nodes)
		{
			// ��map����һ���µ���ʼλ�ã���ԭ���Ľڵ㸴�ƹ�ȥ
			/* 
			���ڼ����µ���ʼλ�ã���map���ҵ�һ���ܹ�ʹ�½ڵ������ж��룺(_map_size - new_num_nodes) / 2��
			���½ڵ�ӳ����е�Ŀ����ƽ�����˿ռ䣬��߲����ɾ��Ч�ʣ�����ӳ��ָ��ʧЧ�ĸ��ʣ���ֹ�ռ��˷ѺͲ���Ҫ���ڴ����
			������add_at_front��ֵ����ǰ����Ҫƫ��nodes_to_add��λ�ã������������á�
			����map�ж�ʮ��λ�ã����λ�ô洢�˽ڵ㣬����Ҫ��ǰ����������ڵ㣬��ʱnew_num_nodes = 8
			��ônew_node_start =_map + (20 - 8) / 2 + (true ? 3 : 0)Ҳ���ǵ�10��λ��
			*/
			new_node_start = _map + (_map_size - new_num_nodes) / 2 +
				(add_at_front ? nodes_to_add : 0);

			// ���ԭ��ʼλ�ô�����λ�ã���ô���ڵ������Ƶ���λ�ã����С����ô��������
			// �������������˵��new_node_start = 10 > 0(_start.node),��ô�Ӻ���ǰ����
			// ��ԭ�����15�ſ�ʼ���Ƶ�10�ţ�ֻ��Ų����ԭ���ݵ�λ��
			if (new_node_start < _start.node)
				copy(_start.node, _finish.node + 1, new_node_start);
			else
				copy_backward(_start.node, _finish.node + 1, new_node_start + old_num_nodes);
		}
		else
		{
			// ����map��ʮ��λ�ã����λ�ô洢�˽ڵ㣬����Ҫ��ǰ����������ڵ㣬��ʱnew_num_nodes = 8
			// ��ʱ���½�һ��map����СΪ_map_size + max(_map_size, nodes_to_add) + 2
			// ��+2��Ŀ������Ϊ�˱���ӳ��ָ��ָ��ӳ��߽磬���µ�����ʧЧ���������������λ����ôӳ��ָ���ڲ���ɾ�������ǲ���Խλ��
			// ��Ȼ�����λ�ã���Ϊ���漰����ֵ����ôֱ�ӽ�ԭmap���Ƶ���map
			// ��new_node_startλ�ã�Ȼ���ͷŵ�ԭmap
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