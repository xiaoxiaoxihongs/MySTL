#pragma once
#include "iterator_base.h"
#include "alloc.h"
#include "construct.h"


namespace MySTL {
	/*
	* list每次插入元素或删除元素就配置或释放一个空间，
	* list是一个双向链表，提供一个BidirectionalIterator，且插入删除时原迭代器不会失效
	*/

	struct __list_node_base
	{
		// 前向指针
		__list_node_base* prev;
		// 后向指针
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

	// list不能使用原生的指针，它的迭代器必须有能力指向list节点，所以需要单独设计
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


		// 指向list节点的普通指针
		node m_node;

		// 构造函数
		__list_iterator() = default;
		__list_iterator(node* x) : __list_iterator_base(x) {}
		__list_iterator(const iterator& x) :__list_iterator_base(x.m_node) {}
		~__list_iterator() = default;


		// 对迭代器取值
		reference operator*() const { return ((node*)m_node)->data; }
		// 对迭代器成员存取的标准操作
		pointer operator->() const { return &(operator*()); }

		// 前置++
		self& operator++() 
		{
			this->incr();
			return *this;
		}

		// 后置++
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

		// 后置--
		self& operator--(int) 
		{
			self temp = *this;
			this->decr();
			return temp;
		}
	};

	// 循环双向链表
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
		// 如果让node指向刻意置于尾端的一个空白节点就能满足STL前闭后开的原则
		link_type node;

	protected:
		// 构造一个节点并传回
		link_type get_node() { return list_node_allocator::allocate(); }
		// 释放一个节点
		void release_node(link_type p) { list_node_allocator::deallocate(p); }

		// 构造并配置一个带元素值的节点
		link_type create_node(const T& x)
		{
			// 未使用异常处理
			link_type p = get_node();
			construct(&p->data, x);
			return p;
		}

		// 构造并配置一个不带元素值的节点
		link_type create_node()
		{
			// 未使用异常处理
			link_type p = get_node();
			construct(&p->data);
			return p;
		}

		// 销毁释放一个节点
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
			// distance并没有重载三个参数的，为什么能这样写？我认为应该是下面的，
			// 但在SGI中也是这样写的，不理解
			result = distance(begin(), end());
			return result;
		}

		size_type max_size() const { return size_type(-1); }
		// 头节点的元素值
		reference front() { return *begin(); }
		const_reference cfront() const { return *cbegin(); }
		// 尾节点的元素值
		reference back() { return *end(); }
		const_reference cback() const { return *cend(); }

		void swap(list<T, Alloc>& x) { std::swap(node, x.node); }
	
		// 在position插入一个节点，内容为x
		iterator insert(iterator position, const T& x)
		{
			link_type temp = create_node(x);
			temp->next = position.m_node;
			temp->prev = position.m_node.prev;

			position.m_node.prev->next = temp;
			position.m_node.prev = temp;
			return temp;
		}
		
		// 默认参数插入
		iterator insert(iterator position) { return insert(position, T()); }
		
		// 插入n个x
		void fill_insert(iterator position, size_type n, const T& x) 
		{
			for (; n > 0; --n)
				insert(position, x);
		}

		// 插入一段T类型的数组
		void insert(iterator position, const T* first, const T* last)
		{
			for (; first != last; ++first)
				insert(position, *first);
		}
		// 插入另一个list中[first, last)的数据
		void insert(iterator position, const_iterator first, const_iterator last)
		{
			for (; first != last; ++first)
				insert(position, *first);
		}
		// 插入n个x到position前面
		void insert(iterator position, size_type n, const T& x) { fill_insert(position, n, x); }

		// 插入到最前面
		void push_front(const T& x) { insert(begin(), x); }
		void push_front() { insert(begin()); }

		// 插入到最后面
		void push_back(const T& x) { insert(end(), x); }
		void push_back() { insert(end()); }

		// 移除迭代器所指节点
		iterator erase(iterator position)
		{
			// 保存摘除节点的前后
			__list_node_base* next_node = link_type(position.m_node)->next;
			__list_node_base* prev_node = position.m_node.prev;

			link_type n = (link_type*)position.m_node;
			// 下一个节点的上一个节点变成prev，上一个节点的下一个变成next
			next_node->prev = prev_node;
			prev_node->next = next_node;

			destroy_node(n);
			return iterator((list_node*)next_node;)
		}

		// 清空链表，还原成初始节点
		void clear();

		// 重整链表，如果实际长度等于新长度，将i到end之后的节点删除，否则就插入直到大小为new_size
		void resize(size_type new_size, const T& x);
		void resize(size_type new_size) { return resize(new_size, T()); }

		// 弹出头节点
		void pop_front() { erase(begin()); }

		// 弹出尾节点
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
		// 将数值为value的删除
		void remove(const T& value);
		// 移除数值连续且相同的元素，直到剩下一个
		void unique();
	protected:
		// 将[first,last)内所有元素移动到position之前
		void transfer(iterator position, iterator first, iterator last);
		
	public:
		// 将不同list的接合到this的position前
		void splice(iterator position, list& x)
		{
			if (!x.empty()) this->transfer(position, x.begin(), x.end());
		}

		// 将i所指元素接合与list之前，可以是同一个list
		void splice(iterator position, list&, iterator target)
		{
			iterator tar_next = target;
			++tar_next;
			if (position == target || position == tar_next) return;
			this->transfer(position, target, tar_next);
		}

		// 将[first, last)的元素接合到position后面，可以是同一个list，但position不能被[first,last)包含
		void splice(iterator position, iterator first, iterator last)
		{
			if (first != last) this->transfer(position, first, last);
		}
	
	public:
		// 将x合并到this中，两个list都必须经过递增排序
		void merge(list<T, Alloc>& x);
		// 将*this的内容逆置
		void reverse();
		// 因为list不能使用算法sort()，所以要自己定义一个sort()
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

		// 恢复链表的原始状态
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
		// 如果新长度比旧长度小，就删除i到原末尾
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
		// 链表为空
		if (first == last) return;
		iterator next = first;
		while (++next != last)
		{
			// 相等就删除
			if (*first == *next)
				erase(first);
			else
				first = next;
			// 修改区段范围
			next = first;
		}
	}

	template<class T, class Alloc>
	void list<T, Alloc>::transfer(iterator position, iterator first, iterator last)
	{
		// 左闭右开
		if (position != last)
		{
			// last前一个节点向后指向position，first前一个节点的向后指向last...
			last.m_node.prev->next = position.m_node;
			first.m_node.prev->next = last.m_node;
			position.m_node.prev->next = first.m_node;

			// 将向前的指针指向挪动后的位置
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

		// list经过递增排序
		while (first1 != last1 && first2 != last2)
		{
			if (*first2 < *first1)
			{
				// 将first2插入到first1之前
				iterator next = first2;
				transfer(first1, first2, ++next);
				first2 = next;
			}
			else
			{
				++first1;
			}
		}
		// 当list1到达结尾但list2还有未插入的数据时
		if (first2 != last2) transfer(last1, first2, last2);
	}

	template<class T, class Alloc>
	void list<T, Alloc>::reverse()
	{
		// 判空，若为空或仅有一个元素则不进行操作
		// 虽然可以用size()来判断，但是比较慢
		if (node->next == node || node->next->next == node) return;
		iterator first = begin();
		++first;

		while (first != end())
		{
			// 将该节点插入到头节点之后
			iterator old = first;
			++first;
			transfer(begin(), old, first);
		}
	}

	template<class T, class Alloc>
	void list<T, Alloc>::sort()
	{
		// 判空
		if (node->next == node || node->next->next == node) return;

		// 用于存放从数组中摘取出来的节点和交换中转
		list<T, Alloc> carry;

		// 用于存放中介数据，counter[0]存放2^(0+1)次方的数据
		// counter[1]存放2^(1+1)次方的数据, counter[2]存放2^(2+1)次方的数据
		// 当counter[x]内存放的数据达到上限时，转移到counter[x+1]中
		list<T, Alloc> counter[64];

		// 用于记录当前可处理的数据个数
		int fill = 0;

		while (!empty())
		{
			// 将*this中的首元素移入carry中
			carry.splice(carry.begin(), *this, begin());
			int i = 0;
			// 当上一层链表满了，转移到下一层中，一层只有为空或不为空的状态，
			// 若为空则将上层填入，此时本层填充一半，再填充时本层满，往下层填充
			while (i < fill && !counter[i].empty())
			{
				// merge将carry合并到counter[i]中，merge的数据是递增的，开始时是单个元素，也算是有序
				counter[i].merge(carry);
				// 再将counter[i]与carry交换
				carry.swap(counter[i++]);
				// 为何不直接carry.merge(counter[i]); ++i;呢？
			}
			// 将carry中的数据交换给counter[i]
			carry.swap(counter[i]);
			// 当存储到fill时，扩充
			if (i == fill) ++fill;
		}

		// 最后合并counter，再将它与原始链表交换
		for (int i = 1; i < fill; ++i)
			counter[i].merge(counter[i - 1]);
		swap(counter[fill - 1]);
		// 例如21，45，1，6这三个数据
		// 先将21摘除放置于carry中，此时i=fill不满足条件
		// 执行carry.swap(counter[i])，此时carry为空，counter[0]有一个数据21，扩充fill进入下一轮
		// 将45摘除放置carry中，此时满足条件进入第二重循环，将carry与counter[0]中的元素递增合并
		// 再交换到carry中，出第二重循环，然后再将carry交换给counter[1]，将fill置为2
		// 将1摘除放置carry中，此时counter[0]链表为空链表，不满足，将1放入counter[0]中
		// 将6摘除放置carry中，此时满足条件进入第二重循环，交换得到carry为 1 6
		// ++i后仍满足条件，1 6 与 21 45 合并成1 6 21 45交换给carry，carry交付给下一个counter
		// 
		// 每执行完while，carry总为空，counter[i]中存放目前sort好的元素，之前的counter也为空
		//
	}



}