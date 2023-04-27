#pragma once
#include"iterator_base.h"
#include"type_traits.h"

namespace MySTL
{
	// ��[first, last)����value���
	template<class ForwardIterator, class T>
	ForwardIterator fill(ForwardIterator first, ForwardIterator last, const T& value)
	{
		for (; first != last; ++first)
			*first = value;
	}

	// ��[first, last)��ǰn��Ԫ�ظ�����ֵ�����صĵ�����ָ����������һ��Ԫ�ص���һ��λ��
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

	// �Լ��趨�ıȽϺ���
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

	// �Լ��趨�ıȽϺ���
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
			//�Ե������Ƿ��ͬ������ѭ���Ƿ��������
			for (; first != last; ++first, ++last)
				*result = *first;
			return result;
		}

		template<class RandomAccessIterator, class OutputIterator, class Distance>
		inline OutputIterator __copy_d(RandomAccessIterator first, RandomAccessIterator last,
			OutputIterator result, Distance*)
		{
			//��n>0�������Ƿ��������
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

		// copy�㷨��[first, last)�����Ԫ�ظ��Ƶ�[result, result+(last-first))������
		// ����һ��������result+(last-first)
		template<class InputIterator, class OutputIterator>
		inline OutputIterator copy(InputIterator first, InputIterator last,
			OutputIterator result)
		{
			// ����SGI������һ�����Ǳ��������ĳЩ����template������
			return __copy_dispatch<InputIterator, OutputIterator>()
				(first, last, result);
		}

		// ��ԭ����char���ͽ������أ�ʹ�õײ�memmove�������ٶȿ�
		inline char* copy(const char* first, const char* last, char* result)
		{
			// memmove��memcpy�������Ǳ�֤���������ȷ
			// ����n��
			memmove(result, first, last - first);
			// ���ؿ�����ԭ���ַ���
			return result + (last - first);
		}

	}


	template<class InputIterator, class OutputIterator, class Distance>
	inline OutputIterator __copy(InputIterator first, InputIterator last,
		OutputIterator result, input_iterator_tag, Distance*)
	{
		//�Ե������Ƿ��ͬ������ѭ���Ƿ��������
		for (; first != last; ++first, ++result)
			*result = *first;
		return result;
	}

	template<class RandomAccessIterator, class OutputIterator, class Distance>
	inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last,
		OutputIterator result, Distance*)
	{
		//��n>0�������Ƿ��������
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


	// SGI��ʹ��define֮��֧��char��short��ԭ�����ͣ�������ֱ�����أ�����д����һ��
#define __MYSTL_DECLARE_COPY_TRIVIAL(_T)						\
	inline _T* copy(const _T* first,const _T* last, _T* result)	\
	{															\
		memmove(result, first, last - first);					\
		return result + (last - first);							\
	}															\
	// ƽ������ֱ��ʹ��memmove���п���
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
	
	// û�ж����޺���ģ��ƫ���ϵ�ı��������÷���
	template<class BidirectionalIterator1, class BidirectionalIterator2, class Distance>
	inline BidirectionalIterator2 __copy_backward(BidirectionalIterator1 first, BidirectionalIterator1 last,
		BidirectionalIterator2 result, bidirectional_iterator_tag, Distance*)
	{
		// �Ӻ���ǰ��ֵ
		while (first != last)
			*--result = *--last;
		return result;
	}

	template<class RandomAccessIterator, class BidirectionalIterator, class Distance>
	inline BidirectionalIterator __copy_backward(RandomAccessIterator first, RandomAccessIterator last,
		BidirectionalIterator result, random_access_iterator_tag, Distance*)
	{
		// �Ӻ���ǰ��ֵ
		for (Distance n = last - first; n > 0; --n)
			*--result = *--last;
		return result;
	}

	// ��[first, last)�����ݴ�result��ʼ��ǰ����
	template<class BidirectionalIterator1, class BidirectionalIterator2>
	inline BidirectionalIterator2 copy_backward(BidirectionalIterator1 first, 
		BidirectionalIterator1 last,BidirectionalIterator2 result) 
	{
		return __copy_backward(first, last, result, ITERATOR_CATEGORY(first), DISTANCE_TYPE(first));
	}
}
