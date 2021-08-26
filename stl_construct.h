#ifndef __SIMPLE_STL_INTERNAL_CONSTRUCT_H
#define __SIMPLE_STL_INTERNAL_CONSTRUCT_H

#include <new> // 使用placement new
#include "./type_traits.h"
#include "./stl_iterator.h"

namespace SimpleSTL
{
	template <class T1, class T2>
	inline void construct(T1 *p, const T2 &value)
	{
		new ((void *)p) T1(value); // placement new; 调用T1::T1(value)
	}

	template <class T1>
	inline void construct(T1 *p)
	{
		new ((void *)p) T1();
	}

	// 以下是 destroy() 第一版本，接受一个指针
	template <class T>
	inline void destroy(T *pointer)
	{
		pointer->~T(); // 调用dtor ~T()
	}

	// 以下是 destroy() 第二版本，接受两个迭代器。此函数设法找出元素类别
	// 进而利用 __type_traits<> 求取最适当措施
	template <class ForwardIterator>
	inline void destroy(ForwardIterator first, ForwardIterator last)
	{
		__destroy(first, last, SimpleSTL::value_type(first));
	}

	// 判断元素的数值型别（value type）是否有 trivial destructor
	template <class ForwardIterator, class T>
	inline void __destroy(ForwardIterator first, ForwardIterator last, T *)
	{
		typedef typename _type_traits<ForwardIterator>::has_trivial_destructor Trivial_destructor;	// 根据这个来判断是否有 trivial destructor
		__destroy_aux(first, last, Trivial_destructor()); // 函数重载来选择下面的两种函数
	}

	// 如果元素的数值型别（value type）有 non-trivial destructor……
	template <class ForwardIterator>
	void __destroy_aux(ForwardIterator first, ForwardIterator last, _false_type)
	{
		for (; first != last; ++first)
			destroy(&*first);
	}

	// 如果元素的数值型别（value type）有 trivial destructor……
	template <class ForwardIterator>
	inline void __destroy_aux(ForwardIterator, ForwardIterator, _true_type) {}

	// 以下是 destroy() 的特化版本
	inline void destroy(char *, char *) {}
	inline void destroy(int *, int *) {}
	inline void destroy(long *, long *) {}
	inline void destroy(float *, float *) {}
	inline void destroy(double *, double *) {}
	inline void destroy(wchar_t *, wchar_t *) {}
}

#endif
