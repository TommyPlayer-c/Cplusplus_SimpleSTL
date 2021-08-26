#ifndef __SIMPLE_STL_ITERATOR_H
#define __SIMPLE_STL_ITERATOR_H

#include <cstddef>

namespace SimpleSTL
{
    //五种迭代器类型
	struct input_iterator_tag{};
	struct output_iterator_tag{};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};
    
    // 自行开发的迭代器最好继承下面这个 iterator
    template <class Category,
            class T,
            class Distance = ptrdiff_t,
            class Pointer = T*,
            class Reference = T&>
    struct iterator {
        typedef Category    iterator_category;
        typedef T           value_type;
        typedef Distance    difference_type;
        typedef Pointer     pointer;
        typedef Reference   reference;
    };              
    
    // "榨汁机" traits
    template<class Iterator>
	struct iterator_traits {
		typedef typename Iterator::iterator_category	iterator_category;
		typedef typename Iterator::value_type			value_type;
		typedef typename Iterator::difference_type		difference_type;
		typedef typename Iterator::pointer				pointer;
		typedef typename Iterator::reference 			reference;
	};

    // 针对原生指针（native pointer）而设计的 traits 偏特化版
    template<class T>
	struct iterator_traits<T*> {
		typedef random_access_iterator_tag 	iterator_category;
		typedef T 							value_type;
		typedef ptrdiff_t 					difference_type;
		typedef T*							pointer;
		typedef T& 							reference;
	};
    
    // 针对原生之 pointer-to-const 而设计的 traits 偏特化版
    template<class T>
	struct iterator_traits<const T*> {
		typedef random_access_iterator_tag 	iterator_category;
		typedef T 							value_type;
		typedef ptrdiff_t 					difference_type;
		typedef const T*					pointer;
		typedef const T& 					reference;
	};

    // 这个函数用于判断某个迭代器的类型（category）
    template<class Iterator>
    inline typename iterator_traits<Iterator>::iterator_category
    iterator_category(const Iterator&) {
      	typedef typename iterator_traits<Iterator>::iterator_category category;
      	return category();
    }
    
    // 这个函数用于判断某个迭代器的 difference_type
    template<class Iterator>
	inline typename iterator_traits<Iterator>::difference_type*
	difference_type(const Iterator&){
	    return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
	}

    // 这个函数用于判断某个迭代器的 value type
    template<class Iterator>
    inline typename iterator_traits<Iterator>::value_type*
    value_type(const Iterator&) {
      	return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
    }

    // 以下是整组的 distance 函数
    template <class _InputIterator, class _Distance>
    inline void __distance(_InputIterator __first, _InputIterator __last,
                           _Distance& __n, input_iterator_tag)
    {
      while (__first != __last) { ++__first; ++__n; }
    }

    template <class _RandomAccessIterator, class _Distance>
    inline void __distance(_RandomAccessIterator __first, 
                           _RandomAccessIterator __last, 
                           _Distance& __n, random_access_iterator_tag)
    {
      __n += __last - __first;
    }

    template <class _InputIterator, class _Distance>
    inline void distance(_InputIterator __first, 
                         _InputIterator __last, _Distance& __n)
    {
      __distance(__first, __last, __n, iterator_category(__first));
    }

    // 以下是整组的 distance 函数
    template <class InputIterator>
    inline typename iterator_traits<InputIterator>::difference_type
    __distance(InputIterator first, InputIterator last, input_iterator_tag)
    {
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last) {
            ++first; ++n;
        }
        return n;
    }

    template <class RandomAccessIterator>
    inline typename iterator_traits<RandomAccessIterator>::difference_type
    __distance(RandomAccessIterator first, RandomAccessIterator last,
               random_access_iterator_tag) {
        return last - first;
    }

    // 对外开放的上层控制接口，使用traits机制从迭代器中推导出其类型。
    template <class InputIterator>
    inline typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last) {
        typedef typename
            iterator_traits<InputIterator>::iterator_category category;
        return __distance(first, last, category());
    }


    // 以下是整组 advance 函数
    template <class InputIterator, class Distance>
    inline void __advance(InputIterator& i, Distance n, input_iterator_tag) {
        while (n--) ++i;
    }

    template <class BidirectionalIterator, class Distance>
    inline void __advance(BidirectionalIterator& i, Distance n, 
                          bidirectional_iterator_tag) {
        if (n >= 0)
            while (n--) ++i;
        else
            while (n++) --i;
    }

    template <class RandomAccessIterator, class Distance>
    inline void __advance(RandomAccessIterator& i, Distance n, 
                          random_access_iterator_tag) {
        i += n;
    }

    // 对外开放的上层控制接口，使用traits机制从迭代器中推导出其类型。
    template <class InputIterator, class Distance>
    inline void advance(InputIterator& i, Distance n) {
        __advance(i, n, iterator_category(i));
    }
} // namespace SimpleSTL

#endif