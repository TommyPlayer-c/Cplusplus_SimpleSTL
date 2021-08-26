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

#ifndef __SIMPLE_STL_MEMORY_H
#define __SIMPLE_STL_MEMORY_H

#include "./stl_alloc.h"
#include "./stl_construct.h"
#include "./stl_uninitialized.h"
#include <new>      // for placement new
#include <cstddef>  // for ptrdiff_t, size_t
#include <cstdlib>  // for exit()
#include <climits>  // for UINT_MAX
#include <iostream> // for cerr
using namespace std;

namespace SimpleSTL
{
    template <class T, class Alloc = SimpleSTL::alloc2>     //默认使用第二级配置器
    class allocator         // STL源码剖析中的 simple_alloc
    {
    public:
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

        static T *allocate(size_t n)
        {
            return 0 == n ? 0 : (T *)Alloc::allocate(n * sizeof(T));
        }
        static T *allocate(void)
        {
            return (T *)Alloc::allocate(sizeof(T));
        }

        static void deallocate(T *p, size_t n)
        {
            if (0 != n)
                Alloc::deallocate(p, n * sizeof(T));
        }
        static void deallocate(T *p)
        {
            Alloc::deallocate(p, sizeof(T));
        }

        void construct(pointer p, const T &value)
        {
            SimpleSTL::construct(p, value);
        }

        void destroy(pointer p)
        {
            SimpleSTL::destroy(p);
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

    template<class T, class Alloc>
	class simple_alloc {
	public:
		static T *allocate(size_t n) 
		    { return 0 == n ? 0 : (T*) Alloc::allocate(n * sizeof(T));}

		static T *allocate(void)
		    { return (T*) Alloc::allocate(sizeof(T)); }

		static void deallocate(T *p, size_t n)
		    { if(0 != n) Alloc::deallocate(p, n * sizeof(T)); }

		static void deallocate(T *p)
		    { Alloc::deallocate(p, sizeof(T));}
	};
}

#endif