#pragma once
#include "alloc.h"
#include "construct.h"
#include "uninitialized.h"


namespace MySTL
{
	/*
	vectorά��һ���������ԵĿռ䣬������ʲôԪ�أ���ָͨ�붼���Ե���vector�ĵ�����ʹ��(vector�����������в�����ָͨ�붼�߱�)
	vector�ṩ����Random Access Iterators
	*/
	template<class T, class Alloc = alloc>
	class vector
	{
	public:
		// ���ڵ�vectorʹ�� using value_type = T
		// using point = typename _Alty_traits::pointer
		typedef T					value_type;
		typedef value_type*			point;
		typedef const value_type*	const_point;
		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;
		typedef value_type&			reference;
		typedef const value_type&	const_reference;
		typedef size_t				size_type;
		typedef ptrdiff_t			difference_type;
	protected:
		// �˴��Ƕ���������
		typedef simple_alloc<value_type, alloc> data_allocator;

		// Ŀǰʹ�õĿռ��ͷ
		iterator start;
		// Ŀǰʹ�õĿռ��β
		iterator finish;
		// Ŀǰ��ʹ�õĿռ��β
		iterator end_of_storage;

		//aux��ʾ���Ǹ�������˼����ʾ���������һ����������
		void insert_aux(iterator position, const T& x);
		void deallocate()
		{
			// ���ͷ��ڴ�ʱ��������������deallocate
			if (start) data_allocator::deallocate(start, end_of_storage - start);
		}

		// ��䲢��ʼ�������n��ֵΪvalue
		void fill_initialize(size_type n, const T& value)
		{
			// ��start��ʼ��䣬��������ʹ�õ�β����ʱ����û�п���
			start = allocate_and_fill(n, value);
			finish = start + n;
			end_of_storage = finish;
		}
		
		iterator allocate_and_fill(size_type n, const T& x)
		{
			// ����n��Ԫ�ؿռ�
			iterator result = data_allocator::allocate(n);
			// ��亯��
			uninitialized_fill_n(result, n, x);
			return result;
		}

	public:
		// ��β�ж�
		iterator begin() { return start; }
		const_iterator begin() const { return start; }
		iterator end() { return finish; }
		const_iterator end() const { return finish; }

		// ��С,ǿ��ת����size_type
		size_type size() const { return size_type(end() - begin()); }
		// size_type(-1)��size_type��С������ģ��T�Ĵ�С���ó��������
		size_type max_size() const { return size_type(-1) / sizeof(T); }

		// ����������(capacity)��ʵ�����õ�ʱ���ȿͻ�Ҫ��Ĵ�һ�㣬�����Ϳռ����õ�ʱ��ɱ�
		// ���������ڵó�������
		size_type capacity() const { return size_type(end_of_storage - begin()); }
		// �����Ƿ�Ϊ��
		bool empte() const { return begin() == end(); }
		// �������ã����ص�n��������
		reference operator[](size_type n) { return *(begin() + n); }
		const_reference operator[](size_type n) const { return *(begin() + n); }

	public:
		// Ĭ�ϵ�
		vector() : start(0), finish(0), end_of_storage(0) {}
		// ����n��value���г�ʼ��
		vector(size_type n, const T& value) { fill_initialize(n, value); }
		vector(int n, const T& value) { fill_initialize(n, value); }
		vector(long n, const T& value) { fill_initialize(n, value); }

		// û�в�����ʹ��Ԫ�ص�Ĭ�Ϲ��캯�����й���
		explicit vector(size_type n) { fill_initialize(n, T()); }

		~vector() { destroy(start, finish); deallocate(); }
	public:
		// ��һ��Ԫ��
		reference front() { return *begin(); }
		// ���һ��Ԫ��endָ�����һ��Ԫ��֮���λ��
		reference back() { return *(end() - 1); }

		// ��Ԫ�ز�������β��
		void push_back(const T& x)
		{
			// ���������пռ�ʱ����ȫ�ֹ��캯������Ԫ�ؼ���β������û�пռ������insert_aux����
			if (finish != end_of_storage)
			{
				construct(finish, x);
				++finish;
			}
			else
			{
				insert_aux(end(), x);
			}
		}

		void pop_back()
		{
			// finishǰ�ƣ�����β��Ԫ�أ�����Ԫ��
			--finish;
			destroy(finish);
		}

		// ���ĳ��λ���ϵ�Ԫ��
		iterator erase(iterator position)
		{
			if (position + 1 != end())
				copy(position + 1, finish, position);
			--finish;
			destroy(finish);
			return position;
		}

		void clear() 
		{
			erase(begin(), end());
		}

		// ����λ�ã�������ֵ
		void insert(iterator positiion, size_type n, const T& x);

		// ���洢�����ݴﵽ����������ʱ���ؿ�һ�����飬��ԭֵ���ƽ�ȥ
		void resize(size_type new_size, const T& x)
		{
			if (new_size < size())
				erase(begin() + new_size, end());
			else
				insert(end(), new_size - size(), x);
		}

		// ���ڵ�vector��emplace_back
		// ������
//		template<class... _Valty>
// 		// declptype(auto)����ָʾ�����ķ�������
//		inline decltype(auto) emplace_back(_Valty&&... _Val)
//		{
//			auto& _My_data = _Mypair._Myval2;
//			pointer& _Mylast = _My_data._Mylast;
//			if (_Mylast != _My_data._Myend)
//			{
//				return _Empalce_back_with_unused_capacity(_STD forward<_Valty>(_Val)...);
//			}
//
//			_Ty& _Result = *_Empalce_reallocate(_Mylast, _STD forward<_Valty>(_Val)...);
//#if _HAS_CXX17
//			return _Result;
//#else // ^^^ _HAS_CXX17 ^^^ // vvv !_HAS_CXX17 vvv
//			(void)_Result;
//#endif // _HAS_CXX17
//		}
//		template <class... _Valty>
//		_CONSTEXPR20_CONTAINER decltype(auto) _Emplace_back_with_unused_capacity(_Valty&&... _Val) {
//			// insert by perfectly forwarding into element at end, provide strong guarantee
//			auto& _My_data = _Mypair._Myval2;
//			pointer& _Mylast = _My_data._Mylast;
//			_STL_INTERNAL_CHECK(_Mylast != _My_data._Myend); // check that we have unused capacity
//			_Alty_traits::construct(_Getal(), _Unfancy(_Mylast), _STD forward<_Valty>(_Val)...);
//			_Orphan_range(_Mylast, _Mylast);
//			_Ty& _Result = *_Mylast;
//			++_Mylast;
//#if _HAS_CXX17
//			return _Result;
//#else // ^^^ _HAS_CXX17 ^^^ // vvv !_HAS_CXX17 vvv
//			(void)_Result;
//#endif // _HAS_CXX17
//		}
	};

	template<class T, class Alloc>
	inline void vector<T, Alloc>::insert_aux(iterator position, const T& x)
	{
		// ���б��ÿռ�
		if (finish != end_of_storage)
		{
			// �ڱ��ÿռ����ʼ������һ��Ԫ�أ�����vector�����һ��Ԫ�ظ�ֵ
			construct(finish, *(finish - 1));
			++finish;
			T x_copy = x;
			// ��finish-1��λ�ÿ�ʼ���ƣ��Ӻ���ǰ���ƣ�Ϊ���ڳ�һ���ռ�
			copy_backward(position, finish - 2, finish - 1);
			// ���ڳ��Ŀռ丳ֵΪx
			*position = x_copy;
		}
		// �ޱ��ÿռ�
		else
		{
			const size_type old_size = size();
			// ��ԭ��СΪ0��������1��Ԫ�ش�С������Ϊ0������������
			const size_type new_size = old_size != 0 ? 2 * old_size : 1;

			iterator new_start = data_allocator::allocate(new_size);
			iterator new_finish = new_start;

			try
			{
				// ��if�Ĳ�����࣬�ֽ�������Ҫ�����Ԫ��λ��ǰ��Ԫ�ؿ�������������
				new_finish = uninitialized_copy(start, position, new_start);
				// Ȼ���ڽ�β����һ����Ԫ�أ��ƶ�������
				construct(new_finish, x);
				++new_finish;
				// �ٽ���������ʣ��Ԫ�ؿ�����ȥ
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...)
			{
				// ���ɹ������پ�������
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, new_size);
				throw;
			}
			// ���پ�����
			destroy(begin(), end());
			// ��vector�����
			deallocate();

			// ����������
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + new_size;
		}
	}
	template<class T, class Alloc>
	inline void vector<T, Alloc>::insert(iterator position, size_type n, const T& x)
	{
		// �������Ԫ�ظ�����Ϊ0ʱ
		if (n != 0)
		{
			// �����ÿռ��������n��Ԫ��ʱ
			if (size_type(end_of_storage - finish) >= n)
			{
				T x_copy = x;
				// �����֮���Ԫ�ظ���
				const size_type elems_after = finish - position;
				// ��¼��ĩλ��
				iterator old_finish = finish;
				// �������֮�������Ԫ�ظ�����������Ԫ�ظ���ʱ
				if (elems_after > n)
				{
					// ��finish֮�����finish֮ǰn��Ԫ�أ�����[finish-n,finish)��䵽[finish, finish+n)
					uninitialized_copy(finish - n, finish, finish);
					// �ƶ�β���
					finish += n;
					// ��Դ����[position, old_finish-n)������䵽[old_finish-n, old_finish)��
					copy_backward(position, old_finish - n, old_finish);
					// ��Ŀ������[position, position+n)����x_copy
					fill(position, position + n, x_copy);

					/*
					* �������̣�����Դ����ĩβ�г�һ�������������ȳ���Ƭ�����ڳ�ʼ�����ÿռ�
					* �ٵ���copy_backward��position��������ݴ�old_finish��ǰ���
					* ����ٵ���fill���������������
					* [�������� | ��������(���治��ע���ÿռ�)] -> uninitialized_copy(finish - n, finish, finish) ->
					* [��������-n | �ն� | n�����ÿռ������] -> copy_backward(position, old_finish - n, old_finish)
					* [���ÿռ�-elems_after | �ն� | elems_after] -> fill(position, position + n, x_copy)
					* [���ÿռ�-elems_after | �������� | elems_after]
					*/
				}
				// ���������Ԫ�ظ���С�ڵ�������Ԫ�ظ���
				else
				{
					// ��[finish, finish+(elems_after))ȫ������x_copy
					uninitialized_fill_n(finish, n - elems_after, x_copy);
					finish += n - elems_after;
					// ��Դ����[position, old_finish)��䵽Ŀ������[finish, finish + (old_finish - position))
					uninitialized_copy(position, old_finish, finish);
					finish += elems_after;
					// ��Ŀ������[position, old_finish)����x_copy
					fill(position, old_finish, x_copy);

					/*
					* �������̣����ڱ��ÿռ����n-elems_after��x_copy��
					* �ٵ���uninitialized_copy��position��old_finish�����ݴ�finish��ǰ���
					* ����ٵ���fill���������������
					* [�������� | ��������(���治��ע���ÿռ�)] -> uninitialized_fill_n(finish, n - elems_after, x_copy) ->
					* [�������� | ���n - elems_after��x] -> uninitialized_copy(position, old_finish, finish)
					* [���ÿռ�-n | �ն� | position��old_finish��Դ����] -> fill(position, position + n, x_copy)
					* [���ÿռ�-n | n��x | elems_after]
					*/

				}
			}
			// �����ÿռ�С������Ԫ�ظ���ʱ
			else
			{
				// ��Ϥ�Ĳ���
				const size_type old_size = size();
				// �¿ռ�Ϊ�ɿռ��������ɳ���+����Ԫ�ظ����е����ֵ
				const size_type new_size = old_size + max(old_size, n);

				iterator new_start = data_allocator::allocator(new_size);
				iterator new_finish = new_start;

				try 
				{
					// �Ƚ�position֮ǰ�ĸ��Ƶ��¿ռ䣬�ٲ���n��x���ٽ�Դ���ݸ��Ƶ�����
					new_finish = uninitialized_copy(start, position, new_start);
					new_finish = uninitialized_fill_n(new_finish, n, x);
					new_finish = uninitialized_copy(position, finish, new_finish);
				}
				catch (...)
				{
					destroy(new_start, new_finish);
					data_allocator::deallocate(new_start, new_size);
					throw;
				}

				destroy(start, finish);
				deallocate();

				start = new_start;
				finish = new_finish;
				end_of_storage = new_start + new_size;
			}
		}
	}
}
	