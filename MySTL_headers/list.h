#pragma once
#include "iterator_base.h"
#include "alloc.h"
#include "construct.h"


namespace MySTL {
	/*
	* listÿ�β���Ԫ�ػ�ɾ��Ԫ�ؾ����û��ͷ�һ���ռ䣬
	* list��һ��˫�������ṩһ��BidirectionalIterator���Ҳ���ɾ��ʱԭ����������ʧЧ
	*/

	struct __list_node_base
	{
		// ǰ��ָ��
		__list_node_base* prev;
		// ����ָ��
		__list_node_base* next;
	};

	template<class T>
	struct __list_node:public __list_node_base
	{
		T data;
	};

	struct __list_iterator_base
	{
		using size_type				= size_t;
		using difference_type		= ptrdiff_t;
		using iterator_category		= bidirectional_iterator_tag;

		__list_node_base* node;

		__list_iterator_base() = default;
		__list_iterator_base(__list_node_base* x) : node(x) {}
		~__list_iterator_base() = default;

		void incr() { node = node->next; }
		void decr() { node = node->prev; }

		bool operator==(const __list_iterator_base& x) const {
			return node == x.node;
		}
		bool operator!=(const __list_iterator_base& x) const {
			return node != x.node;
		}
	};

	// list����ʹ��ԭ����ָ�룬���ĵ���������������ָ��list�ڵ㣬������Ҫ�������
	template<class T, class Ref, class Ptr>
	struct __list_iterator: public __list_iterator_base
	{
		using iterator			= __list_iterator<T, T&, T*>;
		using const_iterator	= __list_iterator<T, const T&, const T*>;
		using self				= __list_iterator<T, Ref, Ptr>;

		using value_type		= T;
		using pointer			= Ptr;
		using reference			= Ref;
		using node				= __list_node<T>;


		// ָ��list�ڵ����ָͨ��
		node m_node;

		// ���캯��
		__list_iterator() = default;
		__list_iterator(node* x) : __list_iterator_base(x) {}
		__list_iterator(const iterator& x) :__list_iterator_base(x.m_node) {}
		~__list_iterator() = default;


		// �Ե�����ȡֵ
		reference operator*() const { return ((node*)m_node)->data; }
		// �Ե�������Ա��ȡ�ı�׼����
		pointer operator->() const { return &(operator*()); }

		// ǰ��++
		self& operator++() 
		{
			this->incr();
			return *this;
		}

		// ����++
		self& operator++(int) 
		{
			self temp = *this;
			this->incr();
			return temp;
		}

		self& operator--() 
		{
			this->decr();
			return *this;
		}

		// ����--
		self& operator--(int) 
		{
			self temp = *this;
			this->decr();
			return temp;
		}
	};

	// ѭ��˫������
	template<class T, class Alloc = alloc>
	class list
	{
	public:
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using iterator = __list_iterator<T, T&, T*>;
		using const_iterator = __list_iterator<T, const T&, const T*>;
	protected:
		// in list 
		using list_node = __list_node<T>;
		using list_node_allocator = simple_alloc<T, Alloc>;
	public:
		using link_type = list_node*;
	protected:
		// �����nodeָ���������β�˵�һ���հ׽ڵ��������STLǰ�պ󿪵�ԭ��
		link_type node;

	protected:
		// ����һ���ڵ㲢����
		link_type get_node() { return list_node_allocator::allocate(); }
		// �ͷ�һ���ڵ�
		void release_node(link_type p) { list_node_allocator::deallocate(p); }

		// ���첢����һ����Ԫ��ֵ�Ľڵ�
		link_type create_node(const T& x)
		{
			// δʹ���쳣����
			link_type p = get_node();
			construct(&p->data, x);
			return p;
		}

		// ���첢����һ������Ԫ��ֵ�Ľڵ�
		link_type create_node()
		{
			// δʹ���쳣����
			link_type p = get_node();
			construct(&p->data);
			return p;
		}

		// �����ͷ�һ���ڵ�
		void destroy_node(link_type p)
		{
			destroy(&p->data);
			release_node(p);
		}
	protected:
		void empty_initialize()
		{
			node = get_node();
			node->next = node;
			node->prev = node;
		}

	public:
		iterator begin() { return (link_type)((*node).next); }
		const_iterator cbegin() const noexcept{ return (link_type)((*node).next); }

		iterator end() { return node; }
		const_iterator cend() const noexcept{ return node; }

		bool empty() const { return node->next == node; }

		size_type size() const
		{
			size_type result = 0;
			//distance(begin(), end(), result);
			// distance��û���������������ģ�Ϊʲô������д������ΪӦ��������ģ�
			// ����SGI��Ҳ������д�ģ������
			result = distance(begin(), end());
			return result;
		}

		size_type max_size() const { return size_type(-1); }
		// ͷ�ڵ��Ԫ��ֵ
		reference front() { return *begin(); }
		const_reference cfront() const { return *cbegin(); }
		// β�ڵ��Ԫ��ֵ
		reference back() { return *end(); }
		const_reference cback() const { return *cend(); }

		void swap(list<T, Alloc>& x) { std::swap(node, x.node); }
	
		// ��position����һ���ڵ㣬����Ϊx
		iterator insert(iterator position, const T& x)
		{
			link_type temp = create_node(x);
			temp->next = position.m_node;
			temp->prev = position.m_node.prev;

			position.m_node.prev->next = temp;
			position.m_node.prev = temp;
			return temp;
		}
		
		// Ĭ�ϲ�������
		iterator insert(iterator position) { return insert(position, T()); }
		
		// ����n��x
		void fill_insert(iterator position, size_type n, const T& x) 
		{
			for (; n > 0; --n)
				insert(position, x);
		}

		// ����һ��T���͵�����
		void insert(iterator position, const T* first, const T* last)
		{
			for (; first != last; ++first)
				insert(position, *first);
		}
		// ������һ��list��[first, last)������
		void insert(iterator position, const_iterator first, const_iterator last)
		{
			for (; first != last; ++first)
				insert(position, *first);
		}
		// ����n��x��positionǰ��
		void insert(iterator position, size_type n, const T& x) { fill_insert(position, n, x); }

		// ���뵽��ǰ��
		void push_front(const T& x) { insert(begin(), x); }
		void push_front() { insert(begin()); }

		// ���뵽�����
		void push_back(const T& x) { insert(end(), x); }
		void push_back() { insert(end()); }

		// �Ƴ���������ָ�ڵ�
		iterator erase(iterator position)
		{
			// ����ժ���ڵ��ǰ��
			__list_node_base* next_node = link_type(position.m_node)->next;
			__list_node_base* prev_node = position.m_node.prev;

			link_type n = (link_type*)position.m_node;
			// ��һ���ڵ����һ���ڵ���prev����һ���ڵ����һ�����next
			next_node->prev = prev_node;
			prev_node->next = next_node;

			destroy_node(n);
			return iterator((list_node*)next_node;)
		}

		// ���������ԭ�ɳ�ʼ�ڵ�
		void clear();

		// �����������ʵ�ʳ��ȵ����³��ȣ���i��end֮��Ľڵ�ɾ��������Ͳ���ֱ����СΪnew_size
		void resize(size_type new_size, const T& x);
		void resize(size_type new_size) { return resize(new_size, T()); }

		// ����ͷ�ڵ�
		void pop_front() { erase(begin()); }

		// ����β�ڵ�
		void pop_back()
		{
			iterator temp = end();
			erase(--temp);
		}

	public:
		list(size_type n, const T& value) 
		{
			empty_initialize();
			this->insert(begin(), n, value);
		}

		explicit list(size_type n) : list(n, T()) {}
		list() :list(0, T()) {}

		list(const T* first, const T* last)
		{
			empty_initialize();
			this->insert(begin(), first, last);
		}

		list(const_iterator first, const_iterator last)
		{
			empty_initialize();
			this->insert(begin(), first, last);
		}

		~list() = default;

	public:
		// ����ֵΪvalue��ɾ��
		void remove(const T& value);
		// �Ƴ���ֵ��������ͬ��Ԫ�أ�ֱ��ʣ��һ��
		void unique();
	protected:
		// ��[first,last)������Ԫ���ƶ���position֮ǰ
		void transfer(iterator position, iterator first, iterator last);
		
	public:
		// ����ͬlist�ĽӺϵ�this��positionǰ
		void splice(iterator position, list& x)
		{
			if (!x.empty()) this->transfer(position, x.begin(), x.end());
		}

		// ��i��ָԪ�ؽӺ���list֮ǰ��������ͬһ��list
		void splice(iterator position, list&, iterator target)
		{
			iterator tar_next = target;
			++tar_next;
			if (position == target || position == tar_next) return;
			this->transfer(position, target, tar_next);
		}

		// ��[first, last)��Ԫ�ؽӺϵ�position���棬������ͬһ��list����position���ܱ�[first,last)����
		void splice(iterator position, iterator first, iterator last)
		{
			if (first != last) this->transfer(position, first, last);
		}
	
	public:
		// ��x�ϲ���this�У�����list�����뾭����������
		void merge(list<T, Alloc>& x);
		// ��*this����������
		void reverse();
		// ��Ϊlist����ʹ���㷨sort()������Ҫ�Լ�����һ��sort()
		void sort();

	};

	template<class T, class Alloc>
	void list<T, Alloc>::clear()
	{
		link_type cur = (link_type)node->next;
		while (cur != node)
		{
			link_type temp = cur;
			cur = (link_type)cur->next;
			destroy_node(temp);
		}

		// �ָ������ԭʼ״̬
		node->next = node;
		node->prev = node;
	}
	template<class T, class Alloc>
	void list<T, Alloc>::resize(size_type new_size, const T& x)
	{
		iterator i = begin();
		size_type len = 0;
		for (; i != end() && len < new_size;)
			++i, ++len;
		// ����³��ȱȾɳ���С����ɾ��i��ԭĩβ
		if (len == new_size)
			erase(i, end());
		else
			insert(end(), new_size - len, x);
	}
	template<class T, class Alloc>
	void list<T, Alloc>::remove(const T& value)
	{
		iterator first = begin();
		iterator last = end();
		while (first != last)
		{
			iterator next = first;
			++next;
			if (*first == value) erase(first);
			first = next;
		}
	}
	template<class T, class Alloc>
	void list<T, Alloc>::unique()
	{
		iterator first = begin();
		iterator last = end();
		// ����Ϊ��
		if (first == last) return;
		iterator next = first;
		while (++next != last)
		{
			// ��Ⱦ�ɾ��
			if (*first == *next)
				erase(first);
			else
				first = next;
			// �޸����η�Χ
			next = first;
		}
	}

	template<class T, class Alloc>
	void list<T, Alloc>::transfer(iterator position, iterator first, iterator last)
	{
		// ����ҿ�
		if (position != last)
		{
			// lastǰһ���ڵ����ָ��position��firstǰһ���ڵ�����ָ��last...
			last.m_node.prev->next = position.m_node;
			first.m_node.prev->next = last.m_node;
			position.m_node.prev->next = first.m_node;

			// ����ǰ��ָ��ָ��Ų�����λ��
			link_type temp = position.m_node.prev;
			position.m_node.prev = last.m_node.prev;
			last.m_node.prev = first.m_node.prev;
			first.m_node.prev = temp;
		}
	}

	template<class T, class Alloc>
	void list<T, Alloc>::merge(list<T, Alloc>& x)
	{
		iterator first1 = begin();
		iterator last1 = end();
		iterator first2 = x.begin();
		iterator last2 = x.end();

		// list������������
		while (first1 != last1 && first2 != last2)
		{
			if (*first2 < *first1)
			{
				// ��first2���뵽first1֮ǰ
				iterator next = first2;
				transfer(first1, first2, ++next);
				first2 = next;
			}
			else
			{
				++first1;
			}
		}
		// ��list1�����β��list2����δ���������ʱ
		if (first2 != last2) transfer(last1, first2, last2);
	}

	template<class T, class Alloc>
	void list<T, Alloc>::reverse()
	{
		// �пգ���Ϊ�ջ����һ��Ԫ���򲻽��в���
		// ��Ȼ������size()���жϣ����ǱȽ���
		if (node->next == node || node->next->next == node) return;
		iterator first = begin();
		++first;

		while (first != end())
		{
			// ���ýڵ���뵽ͷ�ڵ�֮��
			iterator old = first;
			++first;
			transfer(begin(), old, first);
		}
	}

	template<class T, class Alloc>
	void list<T, Alloc>::sort()
	{
		// �п�
		if (node->next == node || node->next->next == node) return;

		// ���ڴ�Ŵ�������ժȡ�����Ľڵ�ͽ�����ת
		list<T, Alloc> carry;

		// ���ڴ���н����ݣ�counter[0]���2^(0+1)�η�������
		// counter[1]���2^(1+1)�η�������, counter[2]���2^(2+1)�η�������
		// ��counter[x]�ڴ�ŵ����ݴﵽ����ʱ��ת�Ƶ�counter[x+1]��
		list<T, Alloc> counter[64];

		// ���ڼ�¼��ǰ�ɴ�������ݸ���
		int fill = 0;

		while (!empty())
		{
			// ��*this�е���Ԫ������carry��
			carry.splice(carry.begin(), *this, begin());
			int i = 0;
			// ����һ���������ˣ�ת�Ƶ���һ���У�һ��ֻ��Ϊ�ջ�Ϊ�յ�״̬��
			// ��Ϊ�����ϲ����룬��ʱ�������һ�룬�����ʱ�����������²����
			while (i < fill && !counter[i].empty())
			{
				// merge��carry�ϲ���counter[i]�У�merge�������ǵ����ģ���ʼʱ�ǵ���Ԫ�أ�Ҳ��������
				counter[i].merge(carry);
				// �ٽ�counter[i]��carry����
				carry.swap(counter[i++]);
				// Ϊ�β�ֱ��carry.merge(counter[i]); ++i;�أ�
			}
			// ��carry�е����ݽ�����counter[i]
			carry.swap(counter[i]);
			// ���洢��fillʱ������
			if (i == fill) ++fill;
		}

		// ���ϲ�counter���ٽ�����ԭʼ������
		for (int i = 1; i < fill; ++i)
			counter[i].merge(counter[i - 1]);
		swap(counter[fill - 1]);
		// ����21��45��1��6����������
		// �Ƚ�21ժ��������carry�У���ʱi=fill����������
		// ִ��carry.swap(counter[i])����ʱcarryΪ�գ�counter[0]��һ������21������fill������һ��
		// ��45ժ������carry�У���ʱ������������ڶ���ѭ������carry��counter[0]�е�Ԫ�ص����ϲ�
		// �ٽ�����carry�У����ڶ���ѭ����Ȼ���ٽ�carry������counter[1]����fill��Ϊ2
		// ��1ժ������carry�У���ʱcounter[0]����Ϊ�����������㣬��1����counter[0]��
		// ��6ժ������carry�У���ʱ������������ڶ���ѭ���������õ�carryΪ 1 6
		// ++i��������������1 6 �� 21 45 �ϲ���1 6 21 45������carry��carry��������һ��counter
		// 
		// ÿִ����while��carry��Ϊ�գ�counter[i]�д��Ŀǰsort�õ�Ԫ�أ�֮ǰ��counterҲΪ��
		//
	}



}