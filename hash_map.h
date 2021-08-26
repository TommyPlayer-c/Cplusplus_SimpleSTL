#ifndef _SIMPLE_STL_HASHMAP_H_
#define _SIMPLE_STL_HASHMAP_H_

#include "./stl_hashtable.h"
#include "memory.h"

namespace SimpleSTL
{
    template <class Key,
              class T,
              class HashFcn = hash<Key>,
              class EqualKey = equal_to<Key>,
              class Alloc = alloc2>
    class hash_map
    {
    private:
        typedef hashtable<pair<const Key, T>, Key, HashFcn, _Select1st<pair<const Key, T> >,
                          EqualKey, Alloc> ht;
        ht rep; // 底层机制以 hash table 完成

    public:
        typedef typename ht::key_type key_type;
        typedef T data_type;
        typedef T mapped_type;
        typedef typename ht::value_type value_type;
        typedef typename ht::hasher hasher;
        typedef typename ht::key_equal key_equal;

        typedef typename ht::size_type size_type;
        typedef typename ht::difference_type difference_type;

        typedef typename ht::const_pointer pointer;
        typedef typename ht::const_pointer const_pointer;
        typedef typename ht::const_reference reference;
        typedef typename ht::const_reference const_reference;

        typedef typename ht::iterator iterator;     // 和set不同，这里就是 iterator
        typedef typename ht::const_iterator const_iterator;

        // typedef typename ht::allocator_type allocator_type;

        hasher hash_funct() const { return rep.hash_funct(); }
        key_equal key_eq() const { return rep.key_eq(); }

    public:
        hash_map()
            : rep(100, hasher(), key_equal()) {}
        explicit hash_map(size_type __n)
            : rep(__n, hasher(), key_equal()) {}
        hash_map(size_type __n, const hasher &__hf)
            : rep(__n, __hf, key_equal()) {}
        hash_map(size_type __n, const hasher &__hf, const key_equal &__eql)
            : rep(__n, __hf, __eql) {}

        // 以下，插入操作全部使用 insert_unique()，不允许键值重复
        template <class _InputIterator>
        hash_map(_InputIterator __f, _InputIterator __l)
            : rep(100, hasher(), key_equal())
        {
            rep.insert_unique(__f, __l);
        }
        template <class _InputIterator>
        hash_map(_InputIterator __f, _InputIterator __l, size_type __n)
            : rep(__n, hasher(), key_equal())
        {
            rep.insert_unique(__f, __l);
        }
        template <class _InputIterator>
        hash_map(_InputIterator __f, _InputIterator __l, size_type __n,
                 const hasher &__hf)
            : rep(__n, __hf, key_equal())
        {
            rep.insert_unique(__f, __l);
        }
        template <class _InputIterator>
        hash_map(_InputIterator __f, _InputIterator __l, size_type __n,
                 const hasher &__hf, const key_equal &__eql)
            : rep(__n, __hf, __eql)
        {
            rep.insert_unique(__f, __l);
        }

    public:
        size_type size() const { return rep.size(); }
        size_type max_size() const { return rep.max_size(); }
        bool empty() const { return rep.empty(); }
        void swap(hash_map &__hs) { rep.swap(__hs.rep); }
        
        // 本来是const
        iterator begin() { return rep.begin(); }
        iterator end() { return rep.end(); }

    public:
        pair<iterator, bool> insert(const value_type &__obj)
        {
            pair<typename ht::iterator, bool> __p = rep.insert_unique(__obj);
            return pair<iterator, bool>(__p.first, __p.second);
        }
        template <class _InputIterator>
        void insert(_InputIterator __f, _InputIterator __l)
        {
            rep.insert_unique(__f, __l);
        }

        pair<iterator, bool> insert_noresize(const value_type &__obj)
        {
            pair<typename ht::iterator, bool> __p =
                rep.insert_unique_noresize(__obj);
            return pair<iterator, bool>(__p.first, __p.second);
        }
        
        // 本来应该是 const，但是只定义了 iterator
        iterator find(const key_type &__key) { return rep.find(__key); }

        size_type count(const key_type &__key) { return rep.count(__key); }

        T& operator[](const key_type& key)
        {
            return rep.find_or_insert(value_type(key, T())).second;
        }
        // pair<iterator, iterator> equal_range(const key_type &__key) const
        // {
        // return rep.equal_range(__key);
        // }

        size_type erase(const key_type &__key) { return rep.erase(__key); }
        void erase(iterator __it) { rep.erase(__it); }
        void erase(iterator __f, iterator __l) { rep.erase(__f, __l); }
        void clear() { rep.clear(); }

    public:
        void resize(size_type __hint) { rep.resize(__hint); }
        size_type bucket_count() const { return rep.bucket_count(); }
        size_type max_bucket_count() const { return rep.max_bucket_count(); }
        size_type elems_in_bucket(size_type __n) const
        {
            return rep.elems_in_bucket(__n);
        }
    };

    template <class Key, class T, class _HashFcn, class _EqualKey, class _Alloc>
    inline bool
    operator==(const hash_map<Key, T, _HashFcn, _EqualKey, _Alloc> &__hs1,
               const hash_map<Key, T, _HashFcn, _EqualKey, _Alloc> &__hs2)
    {
        return __hs1.rep == __hs2.rep;
    }

    template <class Key, class T, class _HashFcn, class _EqualKey, class _Alloc>
    inline bool
    operator!=(const hash_map<Key, T, _HashFcn, _EqualKey, _Alloc> &__hs1,
               const hash_map<Key, T, _HashFcn, _EqualKey, _Alloc> &__hs2)
    {
        return !(__hs1 == __hs2);
    }

    template <class Key, class T, class _HashFcn, class _EqualKey, class _Alloc>
    inline void
    swap(hash_map<Key, T, _HashFcn, _EqualKey, _Alloc> &__hs1,
         hash_map<Key, T, _HashFcn, _EqualKey, _Alloc> &__hs2)
    {
        __hs1.swap(__hs2);
    }
}

#endif