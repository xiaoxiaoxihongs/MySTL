#pragma once



namespace MySTL
{
	/* 
	�����������Ͷ���Ϊ���������ô�
	һ�����ڼ�������أ�������ȡ���ƻ�õ��������ͣ�Ȼ������Ϊ�������뺯�����Լ�������أ�
	ֱ���ڱ����ھ���ɰ汾ѡ�����Ч�ʡ�
	���ǿ���ͨ���̳����Ż�������ֻ�����ݵ��õĺ���������D�̳�B����ô��һ��fun�н���һ��B����Ϊ����ʱ��
	��D��������ɼ̳й�ϵ�Զ����ݵ��õ�fun�ϡ�
	*/
	// �������ͱ�ǿ����ϵ�̳�

	// �����������֧�����������ֻ��
	struct input_iterator_tag {};
	// �����������֧�����������ֻд
	struct output_iterator_tag {};
	// ǰ�������������д�����㷨�ڱ������������д
	struct forward_iterator_tag : public input_iterator_tag {};
	// ˫�������������˫���ƶ���д
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	// ������ʵ���������������ָ������������֧��O(1)ʱ���Ԫ�����λ�÷��ʶ�д
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};

	/*
	���ϣ���Զ���ĵ�����������STL��һ��Ҫ����������Ӧ�ͱ������ֱ�Ӽ̳�std::iteraor
	iterator_category:�������������ֻ����������ֻд��������
	value_type:��������ָ��������
	difference_type:������ʾ������֮��ľ��룬Ҳ����������ʾ�������������
	pointer:ָ���������ָ֮��
	reference:*p���ͱ𣬸��ݵ�������ָ֮���Ƿ�����ı䣬���value type��T����ô*pointӦ����T&��
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


	// �����κγ�Ա��ֻ���ͱ���
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

	// ե֭������ȡ�������������࣬typename���߱�������һ���ͱ�
	// ���¿��Խ��class type����ȡ�����޷����ԭ��ָ����Ϊ������
	// ���Iterator���Լ���value type����ô��ȡ�����ľ���Iterator::value_type
	// ��ԭ��ָ��Ҳ����ͨ���˻��ƣ���ȡ����Ӧ����T::value_type
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

	//ģ���ػ�����ȡԭ��ָ�룬���ڽ����Ƕ����
	template<class T>
	struct iterator_traits<T*>
	{
		typedef random_access_iterator_tag			 iterator_category;
		typedef T									 value_type;
		typedef ptrdiff_t							 difference_type;
		typedef T*									 pointer;
		typedef T&									 reference;
	};

	// ��ȡָ������ָ�룬��Ϊ��������Ļ�����ȡ����ԭ��ָ��������const int*
	// ������һ���޷���ֵ����ʱ����������ʹ�ø�ָ�����д�����ĺ���ûʲô�ã�������ֻҪ����non const����
	template<class T>
	struct iterator_traits<const T*>
	{
		typedef random_access_iterator_tag			 iterator_category;
		typedef T									 value_type;
		typedef ptrdiff_t							 difference_type;
		typedef const T*							 pointer;
		typedef const T&							 reference;
	};

	// ���ھ���ĳ������������,
	template<class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category
		iterator_category(const Iterator&)
	{
		// typedef ����������typename ���߱�����iterator_traits<Iterator>::iterator_category����һ������
		typedef typename iterator_traits<Iterator>::iterator_category categoy;
		// return �ص���һ�����캯���𣿻���˵û�д����������ֻ����һ���������������Ķ�����ֱ�ӱ��tag��
		// �ֻ�������һ������ֵ��iterator_category�ĺ�������
		return categoy();
		// ��һƪ˵�Ƿ��ض�����Ϊ�⼸��ָ�����Ͷ��ǿյģ����й��������������������ִ��ʱ�䣬�����ռ�ÿռ��ָ��С��
	}

	template<class Iterator>
	inline typename iterator_traits<Iterator>::difference_type*
		distance_type(const Iterator&)
	{
		// static-cast<Ŀ������>(ԭ����)תΪһ��iterator��Ӧ�������͵�ָ��
		// ����ֻ��Ҫ�õ��������ͣ�����Ҫ�õ�ָ�룬�����ÿ�
		// distance_type��ֹ���������ǲ���Ĭ�Ϲ��죬����ָ��ֻ��Ҫ�����þ��ܵõ�ԭ���ͣ��ռ�ռ��С
		// ʹ��ָ����Խ�ʡ���ù��캯�����������캯�������������ȿ������Ҳ�ָ��ʵ�������󣬲������Ǳ��Ӱ��
		return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	}

	template<class Iterator>
	inline typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator&)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	// SGI V3.3��stl_iterator_base�ж��������ף�һ����֧��ģ�幤���ģ�һ���Ǳ�������֧��ģ��ʱ���ֶ�һ���������

	// STL������û��typename�����ᱨ need 'typename' before 'T::xxx' because 'T' is a dependent scope�����Ĵ�
	// ��Ҫʹ��typename��ʶǶ�״����������ƣ������ָ�����ܻ�����������壬������ģ�����漰һ��Ƕ�״�������ǰҪ����typename
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

	// ������ⲿʹ��
	template<class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
	distance(InputIterator first, InputIterator end)
	{
		typedef typename iterator_traits<InputIterator>::iterator_category category;
		// ���������Զ�ѡ����ʵĺ���
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
