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

#ifndef __TMPALLOC__
#define __TMPALLOC__

#include <new>      // for placement new
#include <cstddef>  // for ptrdiff_t, size_t
#include <cstdlib>  // for exit()
#include <climits>  // for UINT_MAX
#include <iostream> // for cerr
using namespace std;

// SGI的std::allocator，不要使用这个文件，它只是对new和delet做了一层薄薄的包装而已。
namespace QHN
{
    // allocate真正实现
    template <class T>
    inline T *_allocate(ptrdiff_t size, T *)
    {
        set_new_handler(0);
        T *tmp = (T *)(::operator new((size_t)(size * sizeof(T))));
        if (tmp == 0)
        {
            cerr << "out of memory" << endl;
            exit(1);
        }
        return tmp;
    }

    // deallocate真正实现
    template <class T>
    inline void _deallocate(T *buffer)
    {
        ::operator delete(buffer);
    }

    // 等同于 new(const void*) p) T(x)
    template <class T1, class T2>
    inline void _construct(T1 *p, const T2 &value)
    {
        new (p) T1(value); // placement new. invoke ctor of T1
    }

    // 相当于 ptr->~T()。
    template <class T>
    inline void _destroy(T *ptr)
    {
        ptr->~T();
    }

    template <class T>
    class allocator
    {
    public:
        typedef T value_type;
        typedef T *pointer;
        typedef const T *const_pointer;
        typedef T &reference;
        typedef const T &const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        // rebind allocator of type U
        template <class U>
        struct rebind
        {
            typedef allocator<U> other;
        };

        // hint used for locality. ref. [Austern], p189
        pointer allocate(size_type n, const void *hint = 0)
        {
            return _allocate((difference_type)n, (pointer)0);
        }

        void deallocate(pointer p, size_type n)
        {
            _deallocate(p);
        }

        void construct(pointer p, const T &value)
        {
            _construct(p, value);
        }

        void destroy(pointer p)
        {
            _destroy(p);
        }

        // alloc.address(x)相当于&x
        pointer address(reference x)
        {
            return (pointer)&x;
        }

        // alloc.address(x)相当于&x
        const_pointer const_address(const_reference x)
        {
            return (const_pointer)&x;
        }

        // 传回可成功配置的最大量
        size_type max_size() const
        {
            return size_type(UINT_MAX / sizeof(T));
        }
    };
}
#endif