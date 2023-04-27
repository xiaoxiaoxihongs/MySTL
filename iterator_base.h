#pragma once



namespace MySTL
{
	/* 
	讲迭代器类型定义为类有两个好处
	一是用于激活函数重载，利用萃取机制获得迭代器类型，然后将他作为参数传入函数，以激活函数重载，
	直接在编译期就完成版本选择，提高效率。
	二是可以通过继承来优化掉单纯只做传递调用的函数，例如D继承B，那么当一个fun中接受一个B类作为参数时，
	将D传入可以由继承关系自动传递调用到fun上。
	*/
	// 迭代器型别按强化关系继承

	// 输入迭代器：支持逐个遍历，只读
	struct input_iterator_tag {};
	// 输出迭代器：支持逐个遍历，只写
	struct output_iterator_tag {};
	// 前向迭代器：允许写入型算法在本迭代器区间读写
	struct forward_iterator_tag : public input_iterator_tag {};
	// 双向迭代器：可以双向移动读写
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	// 随机访问迭代器：涵盖所有指针算术能力，支持O(1)时间对元素随机位置访问读写
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};

	/*
	如果希望自定义的迭代器可用于STL，一定要定义物种相应型别，最好是直接继承std::iteraor
	iterator_category:迭代器类别，例如只读迭代器，只写迭代器等
	value_type:迭代器所指对象的类别
	difference_type:用来表示迭代器之间的距离，也可以用来表示容器的最大容量
	pointer:指向迭代器所指之物
	reference:*p的型别，根据迭代器所指之物是否允许改变，如果value type是T，那么*point应该是T&，
	*/
	//struct my_input_iterator: public std::iterator
	//	<std::input_iterator_tag, int, int, const int*, int>
	//{
	//public:
	//	explicit my_input_iterator(int _num = 0) :num(_num) {}
	//	my_input_iterator& operator++() 
	//	{
	//		//... 
	//	}
	//	my_input_iterator operator++(int)
	//	{
	//		//...
	//	}
	//	bool operator==(my_input_iterator it) { return num == it.num; }
	//	bool operator!=(my_input_iterator it) { return !(*this == it); }
	//	reference operator*() const { return num; }

	//	my_input_iterator begin() { return my_input_iterator(0); }
	//	my_input_iterator end() { return my_input_iterator(num); }
	//private:
	//	int num = 0;
	//};


	// 不含任何成员，只是型别定义
	template<class Category, class T, class Distance = ptrdiff_t,
		class Pointer = T*, class Reference = T&>
		struct iterator
	{
		typedef Category	iterator_category;
		typedef T			value_type;
		typedef Distance	difference_type;
		typedef Pointer		pointer;
		typedef Reference	reference;
	};

	// 榨汁机，萃取出迭代器的种类，typename告诉编译器是一个型别，
	// 以下可以解决class type的萃取，但无法完成原生指针作为迭代器
	// 如果Iterator有自己的value type，那么萃取出来的就是Iterator::value_type
	// 而原生指针也可以通过此机制，萃取出相应类型T::value_type
	template<class I>
	struct iterator_traits 
	{
		using iterator_category = typename I::iterator_category;
		// typedef typename I::iterator_category iterator_category;
		typedef typename I::value_type		 value_type;
		typedef typename I::difference_type	 difference_type;
		typedef typename I::pointer			 pointer;
		typedef typename I::reference		 reference;
	};

	//模板特化，萃取原生指针，用于解决内嵌类型
	template<class T>
	struct iterator_traits<T*>
	{
		typedef random_access_iterator_tag			 iterator_category;
		typedef T									 value_type;
		typedef ptrdiff_t							 difference_type;
		typedef T*									 pointer;
		typedef T&									 reference;
	};

	// 萃取指向常量的指针，因为利用上面的机制萃取出的原生指针类型是const int*
	// 而声明一个无法赋值的临时变量？（在使用该指针进行写操作的函数没什么用），我们只要它的non const类型
	template<class T>
	struct iterator_traits<const T*>
	{
		typedef random_access_iterator_tag			 iterator_category;
		typedef T									 value_type;
		typedef ptrdiff_t							 difference_type;
		typedef const T*							 pointer;
		typedef const T&							 reference;
	};

	// 用于决定某个迭代器类型,
	template<class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category
		iterator_category(const Iterator&)
	{
		// typedef 创建别名，typename 告诉编译器iterator_traits<Iterator>::iterator_category这是一个类型
		typedef typename iterator_traits<Iterator>::iterator_category categoy;
		// return 回的是一个构造函数吗？还是说没有创建对象而是只创建一个对象名？创建的对象名直接变成tag吗？
		// 又或是声明一个返回值是iterator_category的函数名？
		return categoy();
		// 有一篇说是返回对象，因为这几种指针类型都是空的，所有构造和析构不会带来额外的执行时间，其对象占用空间比指针小。
	}

	template<class Iterator>
	inline typename iterator_traits<Iterator>::difference_type*
		distance_type(const Iterator&)
	{
		// static-cast<目标类型>(原类型)转为一个iterator对应数据类型的指针
		// 我们只需要用到它的类型，不需要用到指针，所以置空
		// distance_type禁止拷贝、或是不能默认构造，返回指针只需要解引用就能得到原类型，空间占用小
		// 使用指针可以节省调用构造函数、拷贝构造函数、析构函数等开销，且不指向实例化对象，不会造成潜在影响
		return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	}

	template<class Iterator>
	inline typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator&)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	// SGI V3.3的stl_iterator_base中定义了两套，一个是支持模板工作的，一个是编译器不支持模板时，手动一个个定义的

	// STL中这里没加typename，但会报 need 'typename' before 'T::xxx' because 'T' is a dependent scope这样的错
	// 需要使用typename标识嵌套从属类型名称，如果不指出可能会产生解析歧义，所以在模板中涉及一个嵌套从属类型前要加上typename
	template<class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
	__distance(InputIterator first, InputIterator end, input_iterator_tag)
	{
		typename iterator_traits<InputIterator>::difference_type n = 0;

		while (first != end) { ++first; ++n; }
		return n;
	}

	template<class RandomAccessIterator>
	inline typename iterator_traits<RandomAccessIterator>::difference_type
	__distance(RandomAccessIterator first, RandomAccessIterator end, random_access_iterator_tag)
	{
		return end - first;
	}

	// 这个供外部使用
	template<class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
	distance(InputIterator first, InputIterator end)
	{
		typedef typename iterator_traits<InputIterator>::iterator_category category;
		// 利用重载自动选择合适的函数
		return __distance(first, end, category());
	}

	template<class InputIterator, class Distance>
	inline void __advance(InputIterator& it, Distance n, input_iterator_tag)
	{
		while (n--) ++it;
	}

	template<class BidirectionalIterator, class Distance>
	inline void __advance(BidirectionalIterator& bt, Distance n, bidirectional_iterator_tag)
	{
		if (n >= 0)
			while (n--) ++bt;
		else
			while (n++) --bt;
	}

	template<class RandomAccessIterator, class Distance>
	inline void __advance(RandomAccessIterator& rt, Distance n, random_access_iterator_tag)
	{
		rt += n;
	}

	template<class InputIterator, class Distance>
	inline void advance(InputIterator& it, Distance n)
	{
		__advance(it, n, iterator_category(it));
	}

#define ITERATOR_CATEGORY(_i)	iterator_category(_i)
#define DISTANCE_TYPE(_i)		distance_type(_i)
#define VALUE_TYPE(_i)			value_type(_i)
}
