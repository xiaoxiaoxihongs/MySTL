#pragma once
#include "alloc.h"
#include "construct.h"
#include "uninitialized.h"


namespace MySTL
{
	/*
	vector维护一个连续线性的空间，无论是什么元素，普通指针都可以当作vector的迭代器使用(vector迭代器的所有操作普通指针都具备)
	vector提供的是Random Access Iterators
	*/
	template<class T, class Alloc = alloc>
	class vector
	{
	public:
		// 现在的vector使用 using value_type = T
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
		// 此处是二级配置器
		typedef simple_alloc<value_type, alloc> data_allocator;

		// 目前使用的空间的头
		iterator start;
		// 目前使用的空间的尾
		iterator finish;
		// 目前可使用的空间的尾
		iterator end_of_storage;

		//aux表示的是辅助的意思，表示这个函数是一个辅助函数
		void insert_aux(iterator position, const T& x);
		void deallocate()
		{
			// 当释放内存时，调用配置器的deallocate
			if (start) data_allocator::deallocate(start, end_of_storage - start);
		}

		// 填充并初始化，填充n个值为value
		void fill_initialize(size_type n, const T& value)
		{
			// 从start开始填充，并设置已使用的尾，此时容器没有空余
			start = allocate_and_fill(n, value);
			finish = start + n;
			end_of_storage = finish;
		}
		
		iterator allocate_and_fill(size_type n, const T& x)
		{
			// 配置n个元素空间
			iterator result = data_allocator::allocate(n);
			// 填充函数
			uninitialized_fill_n(result, n, x);
			return result;
		}

	public:
		// 首尾判断
		iterator begin() { return start; }
		const_iterator begin() const { return start; }
		iterator end() { return finish; }
		const_iterator end() const { return finish; }

		// 大小,强制转化陈size_type
		size_type size() const { return size_type(end() - begin()); }
		// size_type(-1)是size_type大小，除以模板T的大小，得出最大容量
		size_type max_size() const { return size_type(-1) / sizeof(T); }

		// 容器的容量(capacity)在实际配置的时候会比客户要求的大一点，来降低空间配置的时间成本
		// 本函数用于得出现容量
		size_type capacity() const { return size_type(end_of_storage - begin()); }
		// 容器是否为空
		bool empte() const { return begin() == end(); }
		// 重载引用，返回第n个的数据
		reference operator[](size_type n) { return *(begin() + n); }
		const_reference operator[](size_type n) const { return *(begin() + n); }

	public:
		// 默认的
		vector() : start(0), finish(0), end_of_storage(0) {}
		// 复制n个value进行初始化
		vector(size_type n, const T& value) { fill_initialize(n, value); }
		vector(int n, const T& value) { fill_initialize(n, value); }
		vector(long n, const T& value) { fill_initialize(n, value); }

		// 没有参数，使用元素的默认构造函数进行构造
		explicit vector(size_type n) { fill_initialize(n, T()); }

		~vector() { destroy(start, finish); deallocate(); }
	public:
		// 第一个元素
		reference front() { return *begin(); }
		// 最后一个元素end指向最后一个元素之后的位置
		reference back() { return *(end() - 1); }

		// 将元素插入容器尾部
		void push_back(const T& x)
		{
			// 当容器还有空间时，用全局构造函数构造元素加入尾部，若没有空间则调用insert_aux函数
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
			// finish前移，放弃尾端元素，销毁元素
			--finish;
			destroy(finish);
		}

		// 清除某个位置上的元素
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

		// 插入位置，数量，值
		void insert(iterator positiion, size_type n, const T& x);

		// 当存储的数据达到容量的上限时，重开一个数组，把原值复制进去
		void resize(size_type new_size, const T& x)
		{
			if (new_size < size())
				erase(begin() + new_size, end());
			else
				insert(end(), new_size - size(), x);
		}

		// 现在的vector的emplace_back
		// 参数包
//		template<class... _Valty>
// 		// declptype(auto)可以指示函数的返回类型
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
		// 还有备用空间
		if (finish != end_of_storage)
		{
			// 在备用空间的起始处构造一个元素，并用vector的最后一个元素赋值
			construct(finish, *(finish - 1));
			++finish;
			T x_copy = x;
			// 从finish-1的位置开始复制，从后往前复制，为了腾出一个空间
			copy_backward(position, finish - 2, finish - 1);
			// 将腾出的空间赋值为x
			*position = x_copy;
		}
		// 无备用空间
		else
		{
			const size_type old_size = size();
			// 若原大小为0，则配置1个元素大小；若不为0，则配置两倍
			const size_type new_size = old_size != 0 ? 2 * old_size : 1;

			iterator new_start = data_allocator::allocate(new_size);
			iterator new_finish = new_start;

			try
			{
				// 和if的操作差不多，现将旧容器要插入的元素位置前的元素拷贝到新容器中
				new_finish = uninitialized_copy(start, position, new_start);
				// 然后在结尾构造一个新元素，移动迭代器
				construct(new_finish, x);
				++new_finish;
				// 再将久容器的剩下元素拷贝过去
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...)
			{
				// 不成功就销毁旧新容器
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, new_size);
				throw;
			}
			// 销毁旧容器
			destroy(begin(), end());
			// 本vector定义的
			deallocate();

			// 调整迭代器
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + new_size;
		}
	}
	template<class T, class Alloc>
	inline void vector<T, Alloc>::insert(iterator position, size_type n, const T& x)
	{
		// 当插入的元素个数不为0时
		if (n != 0)
		{
			// 当备用空间个数大于n个元素时
			if (size_type(end_of_storage - finish) >= n)
			{
				T x_copy = x;
				// 插入点之后的元素个数
				const size_type elems_after = finish - position;
				// 记录旧末位置
				iterator old_finish = finish;
				// 当插入点之后的现有元素个数大于新增元素个数时
				if (elems_after > n)
				{
					// 在finish之后插入finish之前n个元素，即将[finish-n,finish)填充到[finish, finish+n)
					uninitialized_copy(finish - n, finish, finish);
					// 移动尾标记
					finish += n;
					// 将源区间[position, old_finish-n)逆向填充到[old_finish-n, old_finish)中
					copy_backward(position, old_finish - n, old_finish);
					// 将目标区间[position, position+n)填充成x_copy
					fill(position, position + n, x_copy);

					/*
					* 整体流程：先在源区域末尾切出一块与待插入区间等长的片段用于初始化备用空间
					* 再调用copy_backward将position后面的数据从old_finish向前填充
					* 最后再调用fill将待插入区间填充
					* [已用区间 | 备用区间(后面不关注备用空间)] -> uninitialized_copy(finish - n, finish, finish) ->
					* [已用区间-n | 空洞 | n：已用空间的数据] -> copy_backward(position, old_finish - n, old_finish)
					* [已用空间-elems_after | 空洞 | elems_after] -> fill(position, position + n, x_copy)
					* [已用空间-elems_after | 插入数据 | elems_after]
					*/
				}
				// 插入点后面的元素个数小于等于新增元素个数
				else
				{
					// 将[finish, finish+(elems_after))全部填充成x_copy
					uninitialized_fill_n(finish, n - elems_after, x_copy);
					finish += n - elems_after;
					// 将源区间[position, old_finish)填充到目标区间[finish, finish + (old_finish - position))
					uninitialized_copy(position, old_finish, finish);
					finish += elems_after;
					// 将目标区间[position, old_finish)填充成x_copy
					fill(position, old_finish, x_copy);

					/*
					* 整体流程：先在备用空间填充n-elems_after个x_copy，
					* 再调用uninitialized_copy将position到old_finish的数据从finish向前填充
					* 最后再调用fill将待插入区间填充
					* [已用区间 | 备用区间(后面不关注备用空间)] -> uninitialized_fill_n(finish, n - elems_after, x_copy) ->
					* [已用区间 | 填充n - elems_after个x] -> uninitialized_copy(position, old_finish, finish)
					* [已用空间-n | 空洞 | position到old_finish个源数据] -> fill(position, position + n, x_copy)
					* [已用空间-n | n个x | elems_after]
					*/

				}
			}
			// 当备用空间小于新增元素个数时
			else
			{
				// 熟悉的操作
				const size_type old_size = size();
				// 新空间为旧空间的两倍或旧长度+新增元素个数中的最大值
				const size_type new_size = old_size + max(old_size, n);

				iterator new_start = data_allocator::allocator(new_size);
				iterator new_finish = new_start;

				try 
				{
					// 先将position之前的复制到新空间，再插入n个x，再将源内容复制到后面
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
	