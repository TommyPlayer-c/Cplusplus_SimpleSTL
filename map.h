/***
* 所有元素会根据元素的键值自动被排序，map的所有元素都是pair，同时拥有实值和键值。pair的第一元素被视为键值
* 第二元素被视为实值。map不允许两个元素拥有相同的键值。
* map 不允许修改键值，但允许修改实值
*/
#ifndef _SIMPLE_STL_MAP_H_
#define _SIMPLE_STL_MAP_H_

#include <functional>
#include "memory.h"
#include "stl_tree.h"

namespace SimpleSTL
{
    template <class Key, class T,
              class Compare = less<Key>,
              class Alloc = alloc2>
    class map
    {
    public:
        typedef Key key_type;   // 键值型别
        typedef T data_type;    // 数据型别
        typedef T mapped_type;
        typedef pair<const Key, T> value_type;      // 元素型别（键值/实值）
        typedef Compare key_compare;    // 键值比较函数

        // 以下定义一个 functor，其作用就是调用 “元素比较函数”
        class value_compare
            : public binary_function<value_type, value_type, bool>
        {
            friend class map<Key, T, Compare, Alloc>;
            protected:
                Compare comp;
                value_compare(Compare c) : comp(c) {}
            public:
                bool operator()(const value_type &x, const value_type &y) const
                {
                    return comp(x.first, y.first);
                }
        };

    private:
        template <class X>
        struct select1st : public unary_function<X, typename X::first_type> {
        	const typename X::first_type& operator()(const X& x) const { return x.first; }
        };
        // 以下定义表述型别（representation type）。
        // 以 map 元素型别（一个 pair）的第一型别，作为 RB-tree 节点的键值型别
        typedef rb_tree<key_type, value_type,
                    select1st<value_type>, key_compare, Alloc> rep_type;
        rep_type t;

    public:
        typedef typename rep_type::pointer pointer;
        typedef typename rep_type::const_pointer const_pointer;
        typedef typename rep_type::reference reference;
        typedef typename rep_type::const_reference const_reference;
        typedef typename rep_type::iterator iterator;
        // map 的 iterator 并不是 const_iterator，允许用户通过迭代器修改元素的实值（value）
        // typedef typename rep_type::const_reverse_iterator reverse_iterator;
        // typedef typename rep_type::const_reverse_iterator const_reverse_iterator;
        typedef typename rep_type::const_iterator const_iterator;
        typedef typename rep_type::size_type size_type;
        typedef typename rep_type::difference_type difference_type;

        map() : t(Compare()) {}
        explicit map(const Compare &comp) : t(comp) {}

        template <class InputIterator>
        map(InputIterator first, InputIterator last)
            : t(Compare()) { t.insert_unique(first, last); }

        template <class InputIterator>
        map(InputIterator first, InputIterator last, const Compare &comp)
            : t(comp) { t.insert_unique(first, last); }

        map(const map<Key, T, Compare, Alloc> &x) : t(x.t) {}
        map<Key, T, Compare, Alloc> &operator=(const map<Key, T, Compare, Alloc> &x)
        {
            t = x.t;
            return *this;
        }

        key_compare key_comp() const { return t.key_comp(); }
        value_compare value_comp() const { return value_compare(t.key_comp()); }
        iterator begin() { return t.begin(); }
        const_iterator begin() const { return t.begin(); }
        iterator end() { return t.end(); }
        const_iterator end() const { return t.end(); }
        //  reverse_iterator rbegin() const {return t.rbegin();}
        // reverse_iterator rend() const {return t.rend();}
        bool empty() const { return t.empty(); }
        size_type size() const { return t.size(); }
        size_type max_size() const { return t.max_size(); }

        T& operator[](const key_type &k)
        {
            return (*((insert(value_type(k, T()))).first)).second;
        }
        void swap(map<Key, T, Compare, Alloc> &x) { t.swap(x.t); }

        //return pair 的值
        pair<iterator, bool> insert(const value_type &x)
        {
            return t.insert_unique(x);
        }

        iterator insert(iterator position, const value_type &x)
        {
            return t.insert_unique(position, x);
        }

        template <class InputIterator>
        void insert(InputIterator first, InputIterator last)
        {
            t.insert_unique(first, last);
        }

        void erase(iterator position) { t.erase(position); }
        size_type erase(const key_type &x) { return t.erase(x); }
        void erase(iterator first, iterator last) { t.erase(first, last); }
        void clear() { t.clear(); }

        iterator find(const key_type &x) { return t.find(x); }
        size_type count(const key_type &x) const { return t.count(x); }
        iterator lower_bound(const key_type &x) const
        {
            return t.lower_bound(x);
        }
        iterator upper_bound(const key_type &x) const
        {
            return t.upper_bound(x);
        }
        pair<iterator, iterator> equal_range(const key_type &x) const
        {
            return t.equal_range(x);
        }

        friend bool operator==(const map<Key, T, Compare, Alloc> &x,
                               const map<Key, T, Compare, Alloc> &y)
        {
            return x.t == y.t;
        }
    };

    template <class Key, class T, class Compare, class Alloc>
    inline bool operator<(const map<Key, T, Compare, Alloc> &x,
                          const map<Key, T, Compare, Alloc> &y)
    {
        return x.t < y.t;
    }
}
#endif