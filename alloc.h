#   define __NODE_ALLOCATOR_LOCK
#   define __NODE_ALLOCATOR_UNLOCK
#   define __NODE_ALLOCATOR_THREADS false

#ifdef _USE_MALLOC
typedef _malloc_alloc_template<0> malloc_alloc;
typedef malloc_alloc alloc;
#endif // _USE_MALLOC

#if 0
#    include<new>
#    define _THROW_BAD_ALLOC throw bad_alloc
#elif !defined(_THROW_BAD_ALLOC)
#    include<iostream>
#	 define _THROW_BAD_ALLOC std::cerr<< "out of memory!" << std::endl; exit(1);
#endif

namespace MySTL 
{
	// inst为了定义多个内容相同但class类型不同的模板？
	// 根据编译模式的不同而适应不同的实现？
	// 使用非型别参数inst可以获得一组不同的静态成员？
	template<int inst>
	class _Malloc_Alloc_Template {
	private:
		// oom是out of memory
		// 用于处理内存不足的情况

		// 分配内存
		static void* oom_malloc(size_t);
		// 重分配
		static void* oom_realloc(void*, size_t);
		static void (*_malloc_alloc_oom_handler)();// 函数指针
	public:
		// 分配n个空间，以字节为单位
		static void* allocate(size_t n)
		{
			// 用指针指向分配出的空间
			void* result = malloc(n);
			if (0 == result) result = oom_malloc(n);//右值是为了防止出现错写成赋值的情况
			return result;
		}

		// 释放分配的内存
		static void* deallocate(void* p, size_t /* n 这个参数似乎没用*/)
		{
			// 释放p指针指向的内存
			free(p);
		}

		// 扩大已分配的内存
		static void* reallocate(void* p, size_t /* old size*/, size_t new_size)
		{
			void* result = realloc(p, new_size);
			if (0 == result) result = oom_realloc(p, new_size);
			return result;
		}

		// 仿真C++的set_new_handler()
		/*这是一个参数是函数指针，返回值也是一个函数指针的函数
		* 从内部来看 void (*f)() 是一个返回值为void、无参数的函数指针，将它看作p
		* static void (*set_malloc_handler(p)) ()
		* 外面来看，它是一个参数为p，返回值为void的函数指针
		* 所以set_malloc_handler的参数为void()(),返回值为void()()
		*/
		static void (*set_malloc_handler(void (*f)())) ()
		{

			void (*old)() = _malloc_alloc_oom_handler;
			_malloc_alloc_oom_handler = f;
			return (old);
			
		}
		// 上面的函数首部利用尾置表达式可以写成这样
		// auto set_malloc_handler(void (*f)()) -> void(*)()
		// 上面的可以写成以下形式
		//typedef void (*PF)(); or using PF = void (*)();
		//static PF set_malloc_handler(PF f)
		//{
		//	 PF old = _malloc_alloc_oom_handler;
		//	 _malloc_alloc_oom_handler = f;
		//	 return (old);
		//}

	};

	template<int inst>
	void (*_Malloc_Alloc_Template<inst>::_malloc_alloc_oom_handler)() = 0;

	template<int inst>
	void* _Malloc_Alloc_Template<inst>::oom_malloc(size_t n)
	{
		void (*my_malloc_handler)();
		void* result;

		// 不断尝试释放、配置
		for (;;)
		{
			my_malloc_handler = _malloc_alloc_oom_handler;
			if (0 == my_malloc_handler) { _THROW_BAD_ALLOC; }
			(*my_malloc_handler)(); // 调用处理例程，试图释放内存
			result = malloc(n);		// 尝试配置内存
			if (result) return (result);
		}
	}

	template<int inst>
	void* _Malloc_Alloc_Template<inst>::oom_realloc(void* p, size_t n)
	{
		void (*my_malloc_handler)();
		void* result;

		for (;;)
		{
			my_malloc_handler = _malloc_alloc_oom_handler;
			if (0 == my_malloc_handler) { _THROW_BAD_ALLOC; }
			(*my_malloc_handler)();
			result = realloc(p, n);
			if (result) return (result);
		}
	}

	typedef _Malloc_Alloc_Template<0> malloc_alloc;


	

	/* 第二级配置器
	* 当区块足够大，交移第一级管理器，小于一定值，就以内存池管理
	* 内存池又称次层配置：每次配置一大块内存，并维护对应的自由链表
	* 下次有相同的内存需求，就直接从FreeList中拔出，释放后回收
	*/
	template<bool threads, int inst>
	class __Default_Alloc_Template
	{
	private:
		// 枚举值全部由大写字面组成，单词通过下划线分割
		enum { __SMALL_OBJECT_MAX_BYTES = 128 };

		// 这里能不能和minSTL写的那种，有不同范围的上调呢？
		enum { __ALIGN = 8 };

		// FreeList的个数
		enum { __NUMBER_OF_FREELIST = __SMALL_OBJECT_MAX_BYTES / __ALIGN };
	private:
		static size_t __round_up(size_t bytes)
		{
			// 上调边界
			/*
			return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1));
			enum { __ALIGN = 8 };

			将有关于__ALIGN拆开来看：
			__ALIGN:00001000
			__ALIGN - 1 : 00001000 - 00000001 = 00000111

			~(__ALIGN - 1) : 11111000(取反)

			再知道一个规则，2^n的低n位都是0
			然后再看((bytes)+__ALIGN - 1)这个式子
			若bytes是8的倍数，那么它的低3位都是0,即xxxxx 000，x代表0或1
			那么((bytes)+__ALIGN - 1)就等于xxxxx 000 + 00000 111
			不关注它的高位，那么结果就是xxxxx 111,而再这个式子和~(__ALIGN - 1)做与操作
			(((bytes)+__ALIGN - 1) & ~(__ALIGN - 1)) : xxxxx 000低三位都是0，不变

			若bytes不是8的倍数，则有8n < bytes < 8(n+1),bytes的第三位不都为0
			即xxxxx xxx，若加上__ALIGN - 1，则低三位会发生进位，
			原本是8n+p，操作后变成8(n+1)+p
			再这个式子和~(__ALIGN - 1)做与操作
			后三位清空,变成8(n+1)
			*/
			return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1));
		}

	private:
		// union的大小是结构体里面最大的那个，其他成员共用此内存
		// 由于使用union，指针的第一个字段，它可被视为一个指针，指向相同形式的另一个FreeList
		// 它的第二个字段FreeList可以被视为一个指针，指向实际区块
		// 一物二用的结果就是不会因为维护链表所必须的指针而造成内存的另一种浪费
		union FreeList
		{
			// 指向下一个区块
			union FreeList* free_list_link;
			// 区块首地址
			char client_data[1];
		};
	private:
		// volatile 防止编译器优化导致出现不明确行为
		static FreeList* volatile free_list[__NUMBER_OF_FREELIST];

		// 查找区块编号
		static size_t __free_list_index(size_t bytes)
		{
			// 这个也很简单，例如传入12，即0000 1100，加上0000 0111等于0001 0011
			// 即16+3 = 19， 19/8 = 2， 2-1即区块位置(从0开始)，又或是可以看成右移，0001 0011右移3个单位
			// 0000 0010 就是2
			return (((bytes)+__ALIGN - 1) / __ALIGN - 1);
		}

		// 返回一个大小为n的对象，并可能加入大小为n的其它区块到free_list
		static void* refill(size_t n);

		// 配置一大块空间，可容纳nFreeList个大小为“size”的区块
		// 如果配置nFreeList有所不便，可能会降低配置数量
		static char* chunk_alloc(size_t size, int& nFreeList);

		// chunk:厚块，组块，大块
		// chunk allocation state: 区块分配状态
		static char* start_free; // 内存池起始位置
		static char* end_free; // 内存池结束位置
		static size_t heap_size;// 堆大小

	public:
		// 空间配置函数
		static void* allocate(size_t n);

		// 空间回收函数
		static void* deallocate(void* p, size_t n);

		// 空间再分配
		static void* reallocate(void* p, size_t old_size, size_t new_size);
	};

	// 初始值设定
	template <bool threads, int inst>
	char* __Default_Alloc_Template<threads, inst>::start_free = 0;

	template <bool threads, int inst>
	char* __Default_Alloc_Template<threads, inst>::end_free = 0;

	template <bool threads, int inst>
	size_t __Default_Alloc_Template<threads, inst>::heap_size = 0;


	template <bool threads, int inst>
	__Default_Alloc_Template<threads, inst>::FreeList* volatile
		__Default_Alloc_Template<threads, inst>::free_list[
			__Default_Alloc_Template<threads, inst>::__NUMBER_OF_FREELIST] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 

	// 编译器生成的是inline版本，我觉得没必要
	template<bool threads, int inst>
	void* __Default_Alloc_Template<threads, inst>::allocate(size_t n)
	{
		// 大于小区块标准就用一级配置器
		if (n > (size_t)__SMALL_OBJECT_MAX_BYTES)
		{
			return (malloc_alloc::allocate(n));
		}
		//流程是找到相应区块，然后将该区块的第一个拔出，然后调整原第二块为第一块，回收有deallocate负责

		// 二级指针
		FreeList* volatile* my_free_list;
		FreeList* result;


		// 找到相应的区块
		my_free_list = free_list + __free_list_index(n);

		// result指向该区块的第一个
		result = *my_free_list;
		
		// 当没有找到，也就是空指针时(应该是空指针)
		if (result == 0)
		{
			// 准备重新装填free_list
			void* r = refill(__round_up(n));
			return r;
		}

		// 调整free_list
		*my_free_list = result->free_list_link;
		return (result);
	}
	
	template<bool threads, int inst>
	void* __Default_Alloc_Template<threads, inst>::deallocate(void* p, size_t n)
	{
		// 如果大于小区块标准就调用一级分配器的
		if (n > (size_t)__SMALL_OBJECT_MAX_BYTES)
		{
			malloc_alloc::deallocate(p, n);
		}
		else
		{
			FreeList* volatile* my_free_list;
			// 对p进行转换
			FreeList* q = (FreeList*)p;
			// 定位到该区块
			my_free_list = free_list + __free_list_index(n);
			// 将q的指向下一个空闲区设置为对应的区块，作为头部
			q->free_list_link = *my_free_list;
			// 再将头节点指向本区块
			*my_free_list = q;
		}
	}
	
	template<bool threads, int inst>
	void* __Default_Alloc_Template<threads, inst>::reallocate(void* p, size_t old_size, size_t new_size)
	{
		if (old_size > (size_t)__SMALL_OBJECT_MAX_BYTES && new_size > (size_t)__SMALL_OBJECT_MAX_BYTES)
		{
			return (malloc_alloc::reallocate(p, old_size,new_size));
		}
		void* result;
		size_t copy_sz;
		if (__round_up(old_size) == __round_up(new_size)) return (p);
		
		result = allocate(new_size);
		copy_sz = new_size > old_size ? old_size : new_size;
		memcpy(result, p, copy_sz);
		deallocate(p, old_size);
		return (result);
	}

	template<bool threads, int inst>
	void* __Default_Alloc_Template<threads, inst>::refill(size_t n)
	{
		// 分配的节点数量
		int number_of_Free_List_Node = 20;
		// 指向chunk_alloc分配的结果
		char* chunk = chunk_alloc(n, number_of_Free_List_Node);

		// number_of_Free_List_Node是地址传入，会改变，若只获得一个区块，那么直接分配给调用者，free_list无新节点
		if (1 == number_of_Free_List_Node) return (chunk);

		FreeList* volatile* my_free_list;
		my_free_list = free_list + __free_list_index(n);

		// 定义用于接收chunk分配结果的变量，当前空闲节点、下一个空闲节点的变量
		FreeList* result, * current_free_list, * next_free_list;
		// 直接返回给客户端
		result = (FreeList*)chunk;

		// 令free_list指向新分配的空间，chunk返回的是内存池空闲空间左起点，n是需求分配的块的大小
		*my_free_list = next_free_list = (FreeList*)(chunk + n);

		// 将各个free_list节点串联起来，从1开始是因为0被分配给客户了
		for (int i = 1;; ++i)
		{
			// 将当前节点设置为下一个节点
			current_free_list = next_free_list;
			// 下一个节点往后移n个单位
			next_free_list = (FreeList*)((char*)next_free_list + n);
			if (number_of_Free_List_Node - 1 == i)
			{
				// 当循环次数等于能设置的节点数时，当前节点的下一个节点置0，否则就将当前节点设置成下一个节点
				current_free_list->free_list_link = 0;
				break;
			}
			else
			{
				current_free_list->free_list_link = next_free_list;
			}
		}
		return (result);
	}

	// nFreeList是pass by reference
	// 从内存池中去空间给freelist使用
	template<bool threads, int inst>
	char* __Default_Alloc_Template<threads, inst>::chunk_alloc(size_t size, int& nFreeList)
	{
		char* result;
		
		size_t total_bytes = size * nFreeList;
		// 内存池剩余空间，是从左边开始分配的，所以声明成左边界
		size_t bytes_left = end_free - start_free;

		// 当内存池完全满足需求时
		if (bytes_left >= total_bytes)
		{
			// 将内存池的左边界向右调整申请的大小
			result = start_free;
			start_free += total_bytes;
			return (result);
		}
		// 当内存池不能完全满足需求，只能供应一个或以上的区块时
		else if (bytes_left >= size)
		{
			// 重新设置能够分配的节点个数，pass by reference
			nFreeList = bytes_left / size;
			// 重新设置总申请空间
			total_bytes = size * nFreeList;
			// 和上面分配一样
			result = start_free;
			start_free += total_bytes;
			return (result);
		}
		// 当内存池一个区块的大小都满足不了时
		else
		{
			// 向heap申请内存，大小为2倍的需求量再加上一个随配置次数增加而增加的附加量
			// 这个附加量是什么？它是我们定义的size_t类型的数，被初始化为0，右移4位等于heap_size/2^4,再向上整，意义？
			// 是为了存储FreeList结构吗？大概不是，附加量就是n个区块，可能是因为防止频繁申请，若是申请的次数越多，就请求更多的内存注入到内存池中
			size_t bytes_required = 2 * total_bytes + __round_up(heap_size >> 4);

			// 试着让内存池的残余零头还有应用价值
			if (bytes_left > 0)
			{
				// 内存池还有些零头，分配给适当的free_list
				// 将这些零头分配给合适大小的free_list，
				// 会出现例如还有12bytes就创建一个8bytes的块
				// 将这个块编入free_list[0]这个区块，再剩余的就没法用了这样的情况呢？
				FreeList* volatile* my_free_list = free_list + __free_list_index(bytes_left);
				((FreeList*)start_free)->free_list_link = *my_free_list;
				*my_free_list = (FreeList*)start_free;
			}
			
			// 申请堆空间
			start_free = (char*)malloc(bytes_required);
			// 申请不到heap空间时，就查看我们手头上面的内存
			if (0 == start_free)
			{
				FreeList* volatile* my_free_list, * p;
				// 在我们拥有的资源里查找未使用且足够大的区块
				for (int i = size; i <= __SMALL_OBJECT_MAX_BYTES; i += __ALIGN)
				{
					my_free_list = free_list + __free_list_index(i);
					// p指向该区块的第一个
					p = *my_free_list;
					// 当有空闲区时
					if (0 != p)
					{
						// 将这个空闲区释放
						*my_free_list = p->free_list_link;
						// 重新设置内存池
						start_free = (char*)p;
						end_free = start_free + i;
						// 递归调用，为了修正nFreeList，并且这样可以让任何残余的零头编入适当的free_list备用
						return (chunk_alloc(size, nFreeList));
					}
				}

				// 如果到处都找不到内存，调用第一级的out-of-memory，看看能不能弄点内存来
				// 可能会抛出异常或改善内存不足的情况
				end_free = 0;
				start_free = (char*)malloc_alloc::allocate(bytes_required);
			}

			// 申请到内存后修改堆大小和末位置
			heap_size += bytes_required;
			end_free = start_free + bytes_required;
			// 仍是递归调用，修正nFreeList
			return (chunk_alloc(size, nFreeList));
		}

	}
	typedef __Default_Alloc_Template<false, 0> alloc;
}
