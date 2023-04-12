#ifdef _USE_MALLOC
typedef _malloc_alloc_template<0> malloc_alloc;
typedef malloc_alloc alloc;
# else
// typedef __Default_Alloc_Template<__NODE_ALLOCATOR_THREADS, 0> alloc;
#endif // _USE_MALLOC

#if 0
#    include<new>
#    define _THROW_BAD_ALLOC throw bad_alloc
#elif !defined(_THROW_BAD_ALLOC)
#    include<iostream>
#	 define _THROW_BAD_ALLOC cerr<< "out of memory!" << endl; exit(1);
#endif

namespace MySTL 
{
	// 第一级配置器
	// STL中写的是template<int inst>, 模板偏特化的问题，不是很懂，
	// 但下面这个是C++11的特性，用...来代表可变模板参数, 我觉得差不多
	// template<class... T>
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
			void* result = malloc(p, new_size);
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
		__Default_Alloc_Template<threads, inst>::free_list[__NUMBER_OF_FREELIST] = 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 

	
	template<bool threads, int inst>
	inline void* __Default_Alloc_Template<threads, inst>::refill(size_t n)
	{
		return nullptr;
	}

	template<bool threads, int inst>
	inline char* __Default_Alloc_Template<threads, inst>::chunk_alloc(size_t size, int& nFreeList)
	{
		return nullptr;
	}

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
		if (n > (size_t)__SMALL_OBJECT_MAX_BYTES)
		{
			malloc_alloc::deallocate(p, n);
			return;
		}

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
	
	template<bool threads, int inst>
	void* __Default_Alloc_Template<threads, inst>::reallocate(void* p, size_t old_size, size_t new_size)
	{
		return nullptr;
	}
	
}
