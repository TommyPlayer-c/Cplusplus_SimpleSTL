#ifndef _SIMPLE_STL_STACK_H_
#define _SIMPLE_STL_STACK_H_

#include <cstddef>
#include "./deque.h"
#include "./memory.h"
#include "./stl_iterator.h"

using namespace SimpleSTL;

template <class T, class Sequence = deque<T>>
class stack
{
    // friend bool operator==<>(const stack&, const stack&);
    // friend bool operator< <>(const stack&, const stack&);
public:
    typedef typename Sequence::value_type value_type;
    typedef typename Sequence::size_type size_type;
    // typedef Sequence container_type;
    typedef typename Sequence::reference reference;
    typedef typename Sequence::const_reference const_reference;

protected:
    Sequence c;     // 底层容器

public:
    // 以下完全利用 Sequence C的操作，完成 stack 的操作
    stack() : c() {}
    explicit stack(const Sequence &s) : c(s) {}

    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    reference top() { return c.back(); }
    const_reference top() const { return c.back(); }
    // stack是末端进，末端出（后进先出）
    void push(const value_type &x) { c.push_back(x); }
    void pop() { c.pop_back(); }
};

/*
// 以下是运算符重载，但是deque并没有定义这些，因为还没有完成相应的算法
template <class T, class Sequence>
bool operator==(const stack<T, Sequence> &x, const stack<T, Sequence> &y)
{
    return x.c == y.c;
}

template <class T, class Sequence>
bool operator<(const stack<T, Sequence> &x, const stack<T, Sequence> &y)
{
    return x.c < y.c;
}


template <class T, class Sequence>
bool operator!=(const stack<T, Sequence> &x, const stack<T, Sequence> &y)
{
    return !(x == y);
}

template <class T, class Sequence>
bool operator>(const stack<T, Sequence> &x, const stack<T, Sequence> &y)
{
    return y < x;
}

template <class T, class Sequence>
bool operator<=(const stack<T, Sequence> &x, const stack<T, Sequence> &y)
{
    return !(y < x);
}

template <class T, class Sequence>
bool operator>=(const stack<T, Sequence> &x, const stack<T, Sequence> &y)
{
    return !(x < y);
}
*/
#endif