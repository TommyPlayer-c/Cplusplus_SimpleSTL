#ifndef _SIMPLE_STL_QUEUE_H_
#define _SIMPLE_STL_QUEUE_H_

#include <cstddef>
#include "./deque.h"
#include "./vector.h"
#include "./memory.h"
#include "./stl_heap.h"
#include "./stl_iterator.h"

using namespace SimpleSTL;

namespace SimpleSTL
{
    template <class T, class Sequence = deque<T>>
    class queue
    {
        // friend bool operator==<>(const queue&, const queue&);
        // friend bool operator< <>(const queue&, const queue&);
    public:
        typedef typename Sequence::value_type value_type;
        typedef typename Sequence::size_type size_type;
        // typedef Sequence container_type;
        typedef typename Sequence::reference reference;
        typedef typename Sequence::const_reference const_reference;

    protected:
        Sequence c; // 底层容器

    public:
        // 以下完全利用 Sequence C的操作，完成 queue 的操作
        queue() : c() {}
        explicit queue(const Sequence &s) : c(s) {}

        bool empty() const { return c.empty(); }
        size_type size() const { return c.size(); }
        reference front() { return c.front(); }
        const_reference front() const { return c.front(); }
        reference back() { return c.back(); }
        const_reference back() const { return c.back(); }
        // queue 四末端进，前端出（先进先出）
        void push(const value_type &x) { c.push_back(x); }
        void pop() { c.pop_front(); }
    };

    template <class T,
              class Sequence = vector<T>,
              class Compare = less<typename Sequence::value_type>>
    class priority_queue
    {
    public:
        typedef typename Sequence::value_type value_type;
        typedef typename Sequence::size_type size_type;
        typedef Sequence container_type;

        typedef typename Sequence::reference reference;
        typedef typename Sequence::const_reference const_reference;

    protected:
        Sequence c;     // 底层容器
        Compare comp;   // 元素大小比较标准

    public:
        priority_queue() : c() {}
        explicit priority_queue(const Compare &x) : c(), comp(x) {}
        priority_queue(const Compare &x, const Sequence &s)
            : c(s), comp(x)
        {
            make_heap(c.begin(), c.end(), comp);
        }


        template <class InputIterator>
        priority_queue(InputIterator first, InputIterator last)
            : c(first, last)
        {
            SimpleSTL::make_heap(c.begin(), c.end(), comp);
        }

        template <class InputIterator>
        priority_queue(InputIterator first,
                       InputIterator last, const Compare &x)
            : c(first, last), comp(x)
        {
            make_heap(c.begin(), c.end(), comp);
        }

        template <class InputIterator>
        priority_queue(InputIterator first, InputIterator last,
                       const Compare &x, const Sequence &s)
            : c(s), comp(x)
        {
            c.insert(c.end(), first, last);
            make_heap(c.begin(), c.end(), comp);
        }

        bool empty() const
        {
            return c.empty();
        }
        size_type size() const { return c.size(); }
        const_reference top() const { return c.front(); }     // 这个成员函数本来应该是 const 的，但过不了编译。
        void push(const value_type &x)
        {
            c.push_back(x);
            push_heap(c.begin(), c.end(), comp);
        }
        void pop()
        {
            pop_heap(c.begin(), c.end(), comp);
            c.pop_back();
        }
    };

    // no equality is provided

}
#endif