/*
 * Copyright (c) 2021
 * TommyPlayer-c, https://github.com/TommyPlayer-c
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  
 *
 */

#ifndef __SIMPLE_STL_INTERNAL_ALLOC_H
#define __SIMPLE_STL_INTERNAL_ALLOC_H

#include <malloc.h>
#include <exception>
#include <new> 

namespace SimpleSTL
{
    /************************ 以下为第一级配置器的实现 ************************/
    class alloc1
    {
    private:
        // 以下函数将用来处理内存不足情况
        // oom: out of memory
        static void *oom_malloc(size_t);
        static void *oom_realloc(void *, size_t);
        static void (*__malloc_alloc_oom_handler)(); // 函数指针

    public:
        static void *allocate(size_t n)
        {
            void *result = malloc(n); // 第一级配置器直接使用 malloc()
            if (0 == result)          // 无法满足需求时，改用 oom_malloc
                result = oom_malloc(n);
            return result;
        }

        static void deallocate(void *p, size_t /* n */)
        {
            free(p); // 第一级配置器直接使用 free()
        }

        static void *reallocate(void *p, size_t /* old_sz */, size_t new_sz)
        {
            void *result = realloc(p, new_sz); // 第一级配置器直接使用 realloc()
            if (0 == result)                   // 无法满足需求时使用oom_realloc()
                result = oom_realloc(p, new_sz);
            return result;
        }

        // 以下模拟 c++ 的 set_new_handler()
        // 你可以通过它指定你自己的 oom handler
        static void (*set_malloc_handler(void (*f)()))() // 该函数参数为函数指针
        {                                                // 返回类型也为函数指针
            void (*__old)() = __malloc_alloc_oom_handler;
            __malloc_alloc_oom_handler = f;
            return (__old);
        }
    };

    // alloc1 out-of-memory handling
    // 静态成员变量（函数指针）初值为0，有待客端设定
    void (*alloc1::__malloc_alloc_oom_handler)() = 0;

    void* alloc1::oom_malloc(size_t n)
    {
        void (*my_malloc_handler)();
        void *result;

        for (;;) // 不断尝试释放、配置、再释放、再配置……
        {
            my_malloc_handler = __malloc_alloc_oom_handler;
            if (0 == my_malloc_handler)
            {
                throw std::bad_alloc();
            }
            (*my_malloc_handler)(); // 调用处理例程，企图释放内存
            result = malloc(n);     // 再次尝试配置内存
            if (result)
                return (result);
        }
    }

    void* alloc1::oom_realloc(void *p, size_t n)
    {
        void (*my_malloc_handler)();
        void *result;

        for (;;)
        { // 同上
            my_malloc_handler = __malloc_alloc_oom_handler;
            if (0 == my_malloc_handler)
            {
                throw std::bad_alloc();
            }
            (*my_malloc_handler)();
            result = realloc(p, n);
            if (result)
                return (result);
        }
    }


    /************************ 以下为第二级配置器的实现 ************************/
    class alloc2
    {
    private:
        enum
        {
            __ALIGN = 8
        };
        enum
        {
            __MAX_BYTES = 128
        };
        enum
        {
            __NFREELISTS = 16
        }; // __MAX_BYTES/__ALIGN
        enum
        {
            __NOBJS = 20
        }; //每次增加的节点数量

        // 自由链表（free-lists）节点构造
        union obj
        {
            union obj *free_list_link; /* 一物二用，obj可被视为指向另一个obj的指针。*/
            char client_data[1];       /* The client sees this. */
        };
        // 16个free_list
        static obj *volatile free_list[__NFREELISTS];   // __NFREELISTS 等于 16

        // ROUND_UP() 将 bytes 上调至 8 的倍数，向上取整至8的倍数（取反加一的逆操作）
        static size_t ROUND_UP(size_t bytes)
        {
            return ((bytes + (size_t)__ALIGN - 1) & ~((size_t)__ALIGN - 1));
        }

        // 以下函数根据区块大小，决定使用第 n 号 free-list。n从0起算。
        static size_t FREELIST_INDEX(size_t bytes)
        {
            return ((bytes + (size_t)__ALIGN - 1) / (size_t)__ALIGN - 1);
        }

        // 返回一个大小为 n的对象，并可能加入大小为 n 的其它区块到  free_list
        static void *refill(size_t n);
        // 配置一大块空间，可容纳 nobjs 个大小为"size"的区块。
        // 如果配置 nobjs个区块有所不便，nobjs可能会降低。
        static char *chunk_alloc(size_t size, int& nobjs);

        // chunk allocation state.
        static char *start_free; // 内存池起始位置。只在 chunk_alloc() 变化
        static char *end_free;   // 内存池结束位置。只在 chunk_alloc() 变化
        static size_t heap_size;
        // 注意，以上静态成员变量须在类外初始化。

    public:
        static void* allocate(size_t n)
        {
            // obj *volatile *my_free_list;
            // obj *result;
            // 大于 128 就调用第一级配置器
            if (n > (size_t)__MAX_BYTES)
            {
                return alloc1::allocate(n);
            }
            // 寻找16个freelist中适当的一个。
            // my_free_list = free_list + FREELIST_INDEX(n);
            // result = *my_free_list;
            int index = FREELIST_INDEX(n);  // 和上述语句等价，但是容易理解。
            obj * result = free_list[index];
            if (result == 0)
            {
                // 没找到可用的free list，准备重新填充free list。
                void *r = refill(ROUND_UP(n));
                return r;
            }
            // 区块自free list 拔出，调整 free list，指向下一个指针
            // *my_free_list = result->free_list_link;
            free_list[index] = result->free_list_link;
            return result;
        }

        /* p 不可以是 0 */
        static void deallocate(void *p, size_t n)
        {
            // obj *q = (obj *)p;
            // obj *volatile *my_free_list;

            // 大于128调用第一级配置器
            if (n > (size_t)__MAX_BYTES)
            {
                alloc1::deallocate(p, n);
                return;
            }
            // 寻找对应的free_list
            // my_free_list = free_list + FREELIST_INDEX(n);
            size_t index = FREELIST_INDEX(n);
            obj* node = static_cast<obj *>(p);
            node->free_list_link = free_list[index];
            // 调整 free list，回收区块，纳入 free list
            // q->free_list_link = *my_free_list;
            // *my_free_list = q;
            free_list[index] = node;    // 相当于将一个节点插入至链表头部以前
        }

        static void* reallocate(void *ptr, size_t old_sz, size_t new_sz) {
            deallocate(ptr, old_sz);
            ptr = allocate(new_sz);
            return ptr;
        }
    };

    /************************ 初始化静态变量 ************************/
    char* alloc2::start_free = 0;
    char* alloc2::end_free = 0;
    size_t alloc2::heap_size = 0;
    alloc2::obj* volatile alloc2::free_list[alloc2::__NFREELISTS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    /************************ 2.2.9 重新填充free lists ************************/
    // 传回一个大小为 n 的对象，并且有时候会为适当的 free list 增加节点.
    // 假设 n 已经适当上调至 8 的倍数。
    void* alloc2::refill(size_t n)
    {
        int nobjs = __NOBJS;
        //调用 chunk_alloc()，尝试取得nobjs个区块做为free list的新节点。
        //注意参数 nobjs 是 pass by reference。
        char *chunk = chunk_alloc(n, nobjs);
        obj *volatile *my_free_list;
        obj *result;
        obj *current_obj;
        obj *next_obj;
        int i;

        // 只获得一个区块，这个区块就分配给带哦勇敢者使用，free_list无新节点
        if (1 == nobjs)
            return (chunk);
        // 否则准备调整free_list，纳入入新节点
        my_free_list = free_list + FREELIST_INDEX(n);

        // 在chunk空间内建立free_list
        result = (obj *)chunk;
        // free_list指向新配置的空间（取自内存池）
        *my_free_list = next_obj = (obj *)(chunk + n);
        // free_list节点串联，
        for (i = 1;; i++)
        { // 从1开始，因为0返回给客户端
            current_obj = next_obj;
            next_obj = (obj *)((char *)next_obj + n);
            if (nobjs - 1 == i)
            {
                current_obj->free_list_link = 0;
                break;
            }
            else
            {
                current_obj->free_list_link = next_obj;
            }
        }
        return (result);
    }

    /************************ 2.2.10 内存池（memory pool） ************************/
    // 从内存池中取空间给 free list 使用，是 chunk_alloc 的工作。
    // 假设 size 已经上调至 8 的倍数。
    // 注意参数 nobjs 是 pass by reference
    char* alloc2::chunk_alloc(size_t size, int &nobjs)
    {
        char *result;
        size_t total_bytes = size * nobjs;
        size_t bytes_left = end_free - start_free;

        if (bytes_left >= total_bytes)
        {
            // 内存池剩余空间完全满足需求量
            result = start_free;
            start_free += total_bytes;
            return (result);
        }
        else if (bytes_left >= size)
        {
            // 内存池剩余空间不能完全满足需求量，但足够供应一个（含）以上的区块。
            nobjs = (int)(bytes_left / size);
            total_bytes = size * nobjs;
            result = start_free;
            start_free += total_bytes;
            return (result);
        }
        else
        {
            // 内存池剩余空间连一个区间的大小都无法提供。
            size_t bytes_to_get =
                2 * total_bytes + ROUND_UP(heap_size >> 4);
            // 以下试着让内存池中的残余零头还有利用价值
            if (bytes_left > 0)
            {
                // 内存池还有一些零头，先配给适当的 free_list
                // 首先寻找适当的 free_list
                obj *volatile *my_free_list =
                    free_list + FREELIST_INDEX(bytes_left);
                // 调整 free_list，将内存池中的残余空间编入。
                ((obj *)start_free)->free_list_link = *my_free_list;
                *my_free_list = (obj *)start_free;
            }

            // 配置heap空间，用来补充内存池
            start_free = (char *)malloc(bytes_to_get);
            if (0 == start_free)
            {
                // heap 空间不足，malloc失败
                size_t i;
                obj *volatile *my_free_list;
                obj *p;
                // Try to make do with what we have.  That can't
                // hurt.  We do not try smaller requests, since that tends
                // to result in disaster on multi-process machines.
                // 以下搜寻适当的free_list，所谓适当是指“尚有未用区块，且区块足够大”之free list
                for (i = size; i <= (size_t)__MAX_BYTES; i += (size_t)__ALIGN)
                {
                    my_free_list = free_list + FREELIST_INDEX(i);
                    p = *my_free_list;
                    if (0 != p)
                    { // free list内尚有未用区块
                        // 调整free_list以释放出未用区块
                        *my_free_list = p->free_list_link;
                        start_free = (char *)p;
                        end_free = start_free + i;
                        // 递归调用自己，为了修正 nobjs。
                        return (chunk_alloc(size, nobjs));
                        //注意，任何残余零头终将被编入适当的free-list备用。
                    }
                }
                end_free = 0; // In case of exception.到处都没内存可用了！
                // 调用第一级配置器，看看 out-of-memory 机制能否尽点力。
                start_free = (char *)alloc1::allocate(bytes_to_get);
                // 这会导致掷出异常（exception），或内存不足的情况获得改善
            }
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;
            // 递归调用自己，为了修正 nobjs。
            return (chunk_alloc(size, nobjs));
        }
    }
}

#endif