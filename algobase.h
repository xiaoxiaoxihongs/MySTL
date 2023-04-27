#pragma once
#include"iterator_base.h"
#include"type_traits.h"

namespace MySTL
{
	// 将[first, last)内用value填充
	template<class ForwardIterator, class T>
	ForwardIterator fill(ForwardIterator first, ForwardIterator last, const T& value)
	{
		for (; first != last; ++first)
			*first = value;
	}

	// 将[first, last)内前n个元素该填新值，返回的迭代器指向被填入的最后一个元素的下一个位置
	template<class OutputIterator, class size, class T>
	OutputIterator fill_n(OutputIterator first, size n, const T& value)
	{
		for (; n > 0; --n, ++first) *first = value;
		return first;
	}

	// max and min
	template<class T>
	inline const T& max(const T& a, const T& b)
	{
		return a < b ? b : a;
	}

	// 自己设定的比较函数
	template<class T, class Compare>
	inline const T& max(const T& a, const T& b, Compare compare)
	{
		return compare(a, b) ? b : a;
	}

	template<class T>
	inline const T& min(const T& a, const T& b)
	{
		return b < a ? b : a;
	}

	// 自己设定的比较函数
	template<class T, class Compare>
	inline const T& min(const T& a, const T& b, Compare compare)
	{
		return compare(b, a) ? b : a;
	}

	// copy
	namespace non_partial_ordering_of_function_templates_copy
	{
		template<class InputIterator, class OutputIterator>
		inline OutputIterator __copy(InputIterator first, InputIterator last,
			OutputIterator result, input_iterator_tag)
		{
			//以迭代器是否等同来决定循环是否继续，慢
			for (; first != last; ++first, ++last)
				*result = *first;
			return result;
		}

		template<class RandomAccessIterator, class OutputIterator, class Distance>
		inline OutputIterator __copy_d(RandomAccessIterator first, RandomAccessIterator last,
			OutputIterator result, Distance*)
		{
			//以n>0来决定是否继续，快
			for (Distance n = last - first; n > 0; --n, ++result, ++first)
				*result = *first;
			return result;
		}

		template<class RandomAccessIterator, class OutputIterator>
		inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last,
			OutputIterator result, random_access_iterator_tag)
		{
			return __copy_d(first, last, result, DISTANCE_TYPE(first));
		}

		template<class T>
		inline T* __copy_trivial(const T* first, const T* last, T* result, __true_type)
		{
			memmove(result, first, sizeof(T) * (last - first));
			return result + (last - first);
		}

		template<class T>
		inline T* __copy_trivial(const T* first, const T* last, T* result, __false_type)
		{
			return __copy_d(first, last, result, (ptrdiff_t*)0);
		}

		template<class InputIterator, class OutputIterator>
		struct __copy_dispatch
		{
			OutputIterator operator() (InputIterator first, InputIterator last,
				OutputIterator result)
			{
				return __copy(first, last, result, ITERATOR_CATEGORY(first));
			}
		};

		template<class T>
		struct __copy_dispatch<T*, T*>
		{
			typedef typename __type_traits<T>::has_trivial_assignment_operator t;
			T* operator() (T* first, T* last, T* result)
			{
				return __copy_trivial(first, last, result, t());
			}
		};

		template<class T>
		struct __copy_dispatch<const T*, T*>
		{
			typedef typename __type_traits<T>::has_trivial_assignment_operator t;
			T* operator() (const T* first, const T* last, T* result)
			{
				return __copy_trivial(first, last, result, t());
			}
		};

		// copy算法将[first, last)区间的元素复制到[result, result+(last-first))区间内
		// 返回一个迭代器result+(last-first)
		template<class InputIterator, class OutputIterator>
		inline OutputIterator copy(InputIterator first, InputIterator last,
			OutputIterator result)
		{
			// 但在SGI里面这一段里是被用作针对某些特殊template所做的
			return __copy_dispatch<InputIterator, OutputIterator>()
				(first, last, result);
		}

		// 对原生的char类型进行重载，使用底层memmove函数，速度快
		inline char* copy(const char* first, const char* last, char* result)
		{
			// memmove比memcpy慢，但是保证拷贝结果正确
			// 复制n个
			memmove(result, first, last - first);
			// 返回拷贝后原生字符串
			return result + (last - first);
		}

	}


	template<class InputIterator, class OutputIterator, class Distance>
	inline OutputIterator __copy(InputIterator first, InputIterator last,
		OutputIterator result, input_iterator_tag, Distance*)
	{
		//以迭代器是否等同来决定循环是否继续，慢
		for (; first != last; ++first, ++result)
			*result = *first;
		return result;
	}

	template<class RandomAccessIterator, class OutputIterator, class Distance>
	inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last,
		OutputIterator result, Distance*)
	{
		//以n>0来决定是否继续，快
		for (Distance n = last - first; n > 0; --n, ++result, ++first)
			*result = *first;
		return result;
	}

	template<class T>
	inline T* __copy_trivial(const T* first, const T* last, T* result, __true_type)
	{
		memmove(result, first, sizeof(T) * (last - first));
		return result + (last - first);
	}

	template<class InputIterator, class OutputIterator>
	inline OutputIterator copy(InputIterator first, InputIterator last,
		OutputIterator result)
	{
		return __copy(first, last, result, ITERATOR_CATEGORY(first), DISTANCE_TYPE(first));
	}


	// SGI中使用define之间支持char、short等原生类型，而不是直接重载，这样写方便一点
#define __MYSTL_DECLARE_COPY_TRIVIAL(_T)						\
	inline _T* copy(const _T* first,const _T* last, _T* result)	\
	{															\
		memmove(result, first, last - first);					\
		return result + (last - first);							\
	}															\
	// 平凡类型直接使用memmove进行拷贝
	__MYSTL_DECLARE_COPY_TRIVIAL(char)
	__MYSTL_DECLARE_COPY_TRIVIAL(unsigned char)
	__MYSTL_DECLARE_COPY_TRIVIAL(signed char)
	__MYSTL_DECLARE_COPY_TRIVIAL(short)
	__MYSTL_DECLARE_COPY_TRIVIAL(unsigned short)
	__MYSTL_DECLARE_COPY_TRIVIAL(int)
	__MYSTL_DECLARE_COPY_TRIVIAL(unsigned int)
	__MYSTL_DECLARE_COPY_TRIVIAL(long)
	__MYSTL_DECLARE_COPY_TRIVIAL(unsigned long)
	__MYSTL_DECLARE_COPY_TRIVIAL(wchar_t)
	__MYSTL_DECLARE_COPY_TRIVIAL(long long)
	__MYSTL_DECLARE_COPY_TRIVIAL(unsigned long long)
	__MYSTL_DECLARE_COPY_TRIVIAL(float)
	__MYSTL_DECLARE_COPY_TRIVIAL(double)
	__MYSTL_DECLARE_COPY_TRIVIAL(long double)
#undef __MYSTL_DECLARE_COPY_TRIVIAL

	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// copy_backward
	
	// 没有定义无函数模板偏序关系的编译器调用方法
	template<class BidirectionalIterator1, class BidirectionalIterator2, class Distance>
	inline BidirectionalIterator2 __copy_backward(BidirectionalIterator1 first, BidirectionalIterator1 last,
		BidirectionalIterator2 result, bidirectional_iterator_tag, Distance*)
	{
		// 从后往前赋值
		while (first != last)
			*--result = *--last;
		return result;
	}

	template<class RandomAccessIterator, class BidirectionalIterator, class Distance>
	inline BidirectionalIterator __copy_backward(RandomAccessIterator first, RandomAccessIterator last,
		BidirectionalIterator result, random_access_iterator_tag, Distance*)
	{
		// 从后往前赋值
		for (Distance n = last - first; n > 0; --n)
			*--result = *--last;
		return result;
	}

	// 将[first, last)的内容从result开始往前复制
	template<class BidirectionalIterator1, class BidirectionalIterator2>
	inline BidirectionalIterator2 copy_backward(BidirectionalIterator1 first, 
		BidirectionalIterator1 last,BidirectionalIterator2 result) 
	{
		return __copy_backward(first, last, result, ITERATOR_CATEGORY(first), DISTANCE_TYPE(first));
	}
}
