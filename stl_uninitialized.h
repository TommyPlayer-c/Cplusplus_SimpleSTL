#ifndef __SIMPLE_STL_INTERNAL_UNINITIALIZED_H
#define __SIMPLE_STL_INTERNAL_UNINITIALIZED_H

#include "./type_traits.h"
#include "./stl_construct.h"
#include "./stl_iterator.h"
#include "./algorithm.h"
#include <cstring>

// 这些函数的结构都极其类似，根据type_traits来调用不同类型的函数。

namespace SimpleSTL
{
    /**************************** uninitialized_copy ****************************/ 
    // Valid if copy construction is equivalent to assignment, and if the
    // destructor is trivial.
    template <class InputIter, class ForwardIter>
    inline ForwardIter __uninitialized_copy_aux(
        InputIter first, InputIter last, ForwardIter result, SimpleSTL::_true_type)
    {
        return SimpleSTL::copy(first, last, result);
    }

    template <class InputIter, class ForwardIter>
    ForwardIter __uninitialized_copy_aux(
        InputIter first, InputIter last, ForwardIter result, _false_type)
    {
        ForwardIter cur = result;
        for (; first != last; ++first, ++cur)
            construct(&*cur, *first);
        return cur;
    }

    template <class InputIter, class ForwardIter, class T>
    inline ForwardIter
    __uninitialized_copy(InputIter first, InputIter last,
                         ForwardIter result, T *)
    {
        typedef typename _type_traits<T>::is_POD_type Is_POD;
        return __uninitialized_copy_aux(first, last, result, Is_POD());
    }

    template <class InputIter, class ForwardIter>
    inline ForwardIter
    uninitialized_copy(InputIter first, InputIter last,
                       ForwardIter result)
    {
        return __uninitialized_copy(first, last, result,
                                    value_type(result));
    }

    /**************************** uninitialized_fill ****************************/ 
    // Valid if copy construction is equivalent to assignment, and if the
    // destructor is trivial.
    template <class ForwardIter, class T>
    inline void
    __uninitialized_fill_aux(ForwardIter first, ForwardIter last,
                             const T &x, _true_type)
    {
        fill(first, last, x);
    }

    template <class ForwardIter, class T>
    void
    __uninitialized_fill_aux(ForwardIter first, ForwardIter last,
                             const T &x, _false_type)
    {
        ForwardIter cur = first;
        for (; cur != last; ++cur)
            construct(&*cur, x);
    }

    template <class ForwardIter, class T>
    inline void __uninitialized_fill(ForwardIter first,
                                     ForwardIter last, const T &x, T *)
    {
        typedef typename _type_traits<T>::is_POD_type Is_POD;
        __uninitialized_fill_aux(first, last, x, Is_POD());
    }

    template <class ForwardIter, class T>
    inline void uninitialized_fill(ForwardIter first,
                                   ForwardIter last,
                                   const T &x)
    {
        __uninitialized_fill(first, last, x, value_type(first));
    }

    /**************************** uninitialized_fill_n ****************************/
    // Valid if copy construction is equivalent to assignment, and if the
    //  destructor is trivial.
    template <class ForwardIter, class size, class T>
    inline ForwardIter
    __uninitialized_fill_n_aux(ForwardIter first, size n,
                               const T &x, _true_type)
    {
        return SimpleSTL::fill_n(first, n, x);
    }

    template <class ForwardIter, class size, class T>
    ForwardIter
    __uninitialized_fill_n_aux(ForwardIter first, size n,
                               const T &x, _false_type)
    {
        ForwardIter cur = first;
        for (; n > 0; --n, ++cur)
            construct(&*cur, x);
        return cur;
    }

    template <class ForwardIter, class size, class T>
    inline ForwardIter
    __uninitialized_fill_n(ForwardIter first, size n, const T &x, T *)
    {
        typedef typename _type_traits<T>::is_POD_type Is_POD;
        return __uninitialized_fill_n_aux(first, n, x, Is_POD());
    }

    template <class ForwardIter, class size, class T>
    inline ForwardIter
    uninitialized_fill_n(ForwardIter first, size n, const T &x)
    {
        return __uninitialized_fill_n(first, n, x, value_type(first));
    }
}

#endif