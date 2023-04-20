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
	// instΪ�˶�����������ͬ��class���Ͳ�ͬ��ģ�壿
	// ���ݱ���ģʽ�Ĳ�ͬ����Ӧ��ͬ��ʵ�֣�
	// ʹ�÷��ͱ����inst���Ի��һ�鲻ͬ�ľ�̬��Ա��
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
			void* result = realloc(p, new_size);
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
		__Default_Alloc_Template<threads, inst>::free_list[
			__Default_Alloc_Template<threads, inst>::__NUMBER_OF_FREELIST] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 

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
		// �������С�����׼�͵���һ����������
		if (n > (size_t)__SMALL_OBJECT_MAX_BYTES)
		{
			malloc_alloc::deallocate(p, n);
		}
		else
		{
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
		// ����Ľڵ�����
		int number_of_Free_List_Node = 20;
		// ָ��chunk_alloc����Ľ��
		char* chunk = chunk_alloc(n, number_of_Free_List_Node);

		// number_of_Free_List_Node�ǵ�ַ���룬��ı䣬��ֻ���һ�����飬��ôֱ�ӷ���������ߣ�free_list���½ڵ�
		if (1 == number_of_Free_List_Node) return (chunk);

		FreeList* volatile* my_free_list;
		my_free_list = free_list + __free_list_index(n);

		// �������ڽ���chunk�������ı�������ǰ���нڵ㡢��һ�����нڵ�ı���
		FreeList* result, * current_free_list, * next_free_list;
		// ֱ�ӷ��ظ��ͻ���
		result = (FreeList*)chunk;

		// ��free_listָ���·���Ŀռ䣬chunk���ص����ڴ�ؿ��пռ�����㣬n���������Ŀ�Ĵ�С
		*my_free_list = next_free_list = (FreeList*)(chunk + n);

		// ������free_list�ڵ㴮����������1��ʼ����Ϊ0��������ͻ���
		for (int i = 1;; ++i)
		{
			// ����ǰ�ڵ�����Ϊ��һ���ڵ�
			current_free_list = next_free_list;
			// ��һ���ڵ�������n����λ
			next_free_list = (FreeList*)((char*)next_free_list + n);
			if (number_of_Free_List_Node - 1 == i)
			{
				// ��ѭ���������������õĽڵ���ʱ����ǰ�ڵ����һ���ڵ���0������ͽ���ǰ�ڵ����ó���һ���ڵ�
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

	// nFreeList��pass by reference
	// ���ڴ����ȥ�ռ��freelistʹ��
	template<bool threads, int inst>
	char* __Default_Alloc_Template<threads, inst>::chunk_alloc(size_t size, int& nFreeList)
	{
		char* result;
		
		size_t total_bytes = size * nFreeList;
		// �ڴ��ʣ��ռ䣬�Ǵ���߿�ʼ����ģ�������������߽�
		size_t bytes_left = end_free - start_free;

		// ���ڴ����ȫ��������ʱ
		if (bytes_left >= total_bytes)
		{
			// ���ڴ�ص���߽����ҵ�������Ĵ�С
			result = start_free;
			start_free += total_bytes;
			return (result);
		}
		// ���ڴ�ز�����ȫ��������ֻ�ܹ�Ӧһ�������ϵ�����ʱ
		else if (bytes_left >= size)
		{
			// ���������ܹ�����Ľڵ������pass by reference
			nFreeList = bytes_left / size;
			// ��������������ռ�
			total_bytes = size * nFreeList;
			// ���������һ��
			result = start_free;
			start_free += total_bytes;
			return (result);
		}
		// ���ڴ��һ������Ĵ�С�����㲻��ʱ
		else
		{
			// ��heap�����ڴ棬��СΪ2�����������ټ���һ�������ô������Ӷ����ӵĸ�����
			// �����������ʲô���������Ƕ����size_t���͵���������ʼ��Ϊ0������4λ����heap_size/2^4,�������������壿
			// ��Ϊ�˴洢FreeList�ṹ�𣿴�Ų��ǣ�����������n�����飬��������Ϊ��ֹƵ�����룬��������Ĵ���Խ�࣬�����������ڴ�ע�뵽�ڴ����
			size_t bytes_required = 2 * total_bytes + __round_up(heap_size >> 4);

			// �������ڴ�صĲ�����ͷ����Ӧ�ü�ֵ
			if (bytes_left > 0)
			{
				// �ڴ�ػ���Щ��ͷ��������ʵ���free_list
				// ����Щ��ͷ��������ʴ�С��free_list��
				// ��������绹��12bytes�ʹ���һ��8bytes�Ŀ�
				// ����������free_list[0]������飬��ʣ��ľ�û����������������أ�
				FreeList* volatile* my_free_list = free_list + __free_list_index(bytes_left);
				((FreeList*)start_free)->free_list_link = *my_free_list;
				*my_free_list = (FreeList*)start_free;
			}
			
			// ����ѿռ�
			start_free = (char*)malloc(bytes_required);
			// ���벻��heap�ռ�ʱ���Ͳ鿴������ͷ������ڴ�
			if (0 == start_free)
			{
				FreeList* volatile* my_free_list, * p;
				// ������ӵ�е���Դ�����δʹ�����㹻�������
				for (int i = size; i <= __SMALL_OBJECT_MAX_BYTES; i += __ALIGN)
				{
					my_free_list = free_list + __free_list_index(i);
					// pָ�������ĵ�һ��
					p = *my_free_list;
					// ���п�����ʱ
					if (0 != p)
					{
						// ������������ͷ�
						*my_free_list = p->free_list_link;
						// ���������ڴ��
						start_free = (char*)p;
						end_free = start_free + i;
						// �ݹ���ã�Ϊ������nFreeList�����������������κβ������ͷ�����ʵ���free_list����
						return (chunk_alloc(size, nFreeList));
					}
				}

				// ����������Ҳ����ڴ棬���õ�һ����out-of-memory�������ܲ���Ū���ڴ���
				// ���ܻ��׳��쳣������ڴ治������
				end_free = 0;
				start_free = (char*)malloc_alloc::allocate(bytes_required);
			}

			// ���뵽�ڴ���޸ĶѴ�С��ĩλ��
			heap_size += bytes_required;
			end_free = start_free + bytes_required;
			// ���ǵݹ���ã�����nFreeList
			return (chunk_alloc(size, nFreeList));
		}

	}
	typedef __Default_Alloc_Template<false, 0> alloc;
}
