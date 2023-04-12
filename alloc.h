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
	// ��һ��������
	// STL��д����template<int inst>, ģ��ƫ�ػ������⣬���Ǻܶ���
	// �����������C++11�����ԣ���...������ɱ�ģ�����, �Ҿ��ò��
	// template<class... T>
	template<int inst>
	class _Malloc_Alloc_Template {
	private:
		// oom��out of memory
		// ���ڴ����ڴ治������

		// �����ڴ�
		static void* oom_malloc(size_t);
		// �ط���
		static void* oom_realloc(void*, size_t);
		static void (*_malloc_alloc_oom_handler)();// ����ָ��
	public:
		// ����n���ռ䣬���ֽ�Ϊ��λ
		static void* allocate(size_t n)
		{
			// ��ָ��ָ�������Ŀռ�
			void* result = malloc(n);
			if (0 == result) result = oom_malloc(n);//��ֵ��Ϊ�˷�ֹ���ִ�д�ɸ�ֵ�����
			return result;
		}

		// �ͷŷ�����ڴ�
		static void* deallocate(void* p, size_t /* n ��������ƺ�û��*/)
		{
			// �ͷ�pָ��ָ����ڴ�
			free(p);
		}

		// �����ѷ�����ڴ�
		static void* reallocate(void* p, size_t /* old size*/, size_t new_size)
		{
			void* result = malloc(p, new_size);
			if (0 == result) result = oom_realloc(p, new_size);
			return result;
		}

		// ����C++��set_new_handler()
		/*����һ�������Ǻ���ָ�룬����ֵҲ��һ������ָ��ĺ���
		* ���ڲ����� void (*f)() ��һ������ֵΪvoid���޲����ĺ���ָ�룬��������p
		* static void (*set_malloc_handler(p)) ()
		* ��������������һ������Ϊp������ֵΪvoid�ĺ���ָ��
		* ����set_malloc_handler�Ĳ���Ϊvoid()(),����ֵΪvoid()()
		*/
		static void (*set_malloc_handler(void (*f)())) ()
		{

			void (*old)() = _malloc_alloc_oom_handler;
			_malloc_alloc_oom_handler = f;
			return (old);
		}
		// ����ĺ����ײ�����β�ñ��ʽ����д������
		// auto set_malloc_handler(void (*f)()) -> void(*)()
		// ����Ŀ���д��������ʽ
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

		// ���ϳ����ͷš�����
		for (;;)
		{
			my_malloc_handler = _malloc_alloc_oom_handler;
			if (0 == my_malloc_handler) { _THROW_BAD_ALLOC; }
			(*my_malloc_handler)(); // ���ô������̣���ͼ�ͷ��ڴ�
			result = malloc(n);		// ���������ڴ�
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


	

	/* �ڶ���������
	* �������㹻�󣬽��Ƶ�һ����������С��һ��ֵ�������ڴ�ع���
	* �ڴ���ֳƴβ����ã�ÿ������һ����ڴ棬��ά����Ӧ����������
	* �´�����ͬ���ڴ����󣬾�ֱ�Ӵ�FreeList�аγ����ͷź����
	*/
	template<bool threads, int inst>
	class __Default_Alloc_Template
	{
	private:
		// ö��ֵȫ���ɴ�д������ɣ�����ͨ���»��߷ָ�
		enum { __SMALL_OBJECT_MAX_BYTES = 128 };

		// �����ܲ��ܺ�minSTLд�����֣��в�ͬ��Χ���ϵ��أ�
		enum { __ALIGN = 8 };

		// FreeList�ĸ���
		enum { __NUMBER_OF_FREELIST = __SMALL_OBJECT_MAX_BYTES / __ALIGN };
	private:
		static size_t __round_up(size_t bytes)
		{
			// �ϵ��߽�
			/*
			return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1));
			enum { __ALIGN = 8 };

			���й���__ALIGN��������
			__ALIGN:00001000
			__ALIGN - 1 : 00001000 - 00000001 = 00000111

			~(__ALIGN - 1) : 11111000(ȡ��)

			��֪��һ������2^n�ĵ�nλ����0
			Ȼ���ٿ�((bytes)+__ALIGN - 1)���ʽ��
			��bytes��8�ı�������ô���ĵ�3λ����0,��xxxxx 000��x����0��1
			��ô((bytes)+__ALIGN - 1)�͵���xxxxx 000 + 00000 111
			����ע���ĸ�λ����ô�������xxxxx 111,�������ʽ�Ӻ�~(__ALIGN - 1)�������
			(((bytes)+__ALIGN - 1) & ~(__ALIGN - 1)) : xxxxx 000����λ����0������

			��bytes����8�ı���������8n < bytes < 8(n+1),bytes�ĵ���λ����Ϊ0
			��xxxxx xxx��������__ALIGN - 1�������λ�ᷢ����λ��
			ԭ����8n+p����������8(n+1)+p
			�����ʽ�Ӻ�~(__ALIGN - 1)�������
			����λ���,���8(n+1)
			*/
			return (((bytes)+__ALIGN - 1) & ~(__ALIGN - 1));
		}
		
	private:
		// union�Ĵ�С�ǽṹ�����������Ǹ���������Ա���ô��ڴ�
		// ����ʹ��union��ָ��ĵ�һ���ֶΣ����ɱ���Ϊһ��ָ�룬ָ����ͬ��ʽ����һ��FreeList
		// ���ĵڶ����ֶ�FreeList���Ա���Ϊһ��ָ�룬ָ��ʵ������
		// һ����õĽ�����ǲ�����Ϊά�������������ָ�������ڴ����һ���˷�
		union FreeList
		{
			// ָ����һ������
			union FreeList* free_list_link;
			// �����׵�ַ
			char client_data[1];
		};
	private:
		// volatile ��ֹ�������Ż����³��ֲ���ȷ��Ϊ
		static FreeList* volatile free_list[__NUMBER_OF_FREELIST];

		// ����������
		static size_t __free_list_index(size_t bytes)
		{
			// ���Ҳ�ܼ򵥣����紫��12����0000 1100������0000 0111����0001 0011
			// ��16+3 = 19�� 19/8 = 2�� 2-1������λ��(��0��ʼ)���ֻ��ǿ��Կ������ƣ�0001 0011����3����λ
			// 0000 0010 ����2
			return (((bytes)+__ALIGN - 1) / __ALIGN - 1);
		}

		// ����һ����СΪn�Ķ��󣬲����ܼ����СΪn���������鵽free_list
		static void* refill(size_t n);

		// ����һ���ռ䣬������nFreeList����СΪ��size��������
		// �������nFreeList�������㣬���ܻή����������
		static char* chunk_alloc(size_t size, int& nFreeList);

		// chunk:��飬��飬���
		// chunk allocation state: �������״̬
		static char* start_free; // �ڴ����ʼλ��
		static char* end_free; // �ڴ�ؽ���λ��
		static size_t heap_size;// �Ѵ�С

	public:
		// �ռ����ú���
		static void* allocate(size_t n);

		// �ռ���պ���
		static void* deallocate(void* p, size_t n);

		// �ռ��ٷ���
		static void* reallocate(void* p, size_t old_size, size_t new_size);
	};

	// ��ʼֵ�趨
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

	// ���������ɵ���inline�汾���Ҿ���û��Ҫ
	template<bool threads, int inst>
	void* __Default_Alloc_Template<threads, inst>::allocate(size_t n)
	{
		// ����С�����׼����һ��������
		if (n > (size_t)__SMALL_OBJECT_MAX_BYTES)
		{
			return (malloc_alloc::allocate(n));
		}
		//�������ҵ���Ӧ���飬Ȼ�󽫸�����ĵ�һ���γ���Ȼ�����ԭ�ڶ���Ϊ��һ�飬������deallocate����

		// ����ָ��
		FreeList* volatile* my_free_list;
		FreeList* result;


		// �ҵ���Ӧ������
		my_free_list = free_list + __free_list_index(n);

		// resultָ�������ĵ�һ��
		result = *my_free_list;

		// ��û���ҵ���Ҳ���ǿ�ָ��ʱ(Ӧ���ǿ�ָ��)
		if (result == 0)
		{
			// ׼������װ��free_list
			void* r = refill(__round_up(n));
			return r;
		}

		// ����free_list
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
		// ��p����ת��
		FreeList* q = (FreeList*)p;
		// ��λ��������
		my_free_list = free_list + __free_list_index(n);
		// ��q��ָ����һ������������Ϊ��Ӧ�����飬��Ϊͷ��
		q->free_list_link = *my_free_list;
		// �ٽ�ͷ�ڵ�ָ������
		*my_free_list = q;
	}
	
	template<bool threads, int inst>
	void* __Default_Alloc_Template<threads, inst>::reallocate(void* p, size_t old_size, size_t new_size)
	{
		return nullptr;
	}
	
}
