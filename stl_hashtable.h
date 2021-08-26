#ifndef _SIMPLE_STL_HASHTABLE_H_
#define _SIMPLE_STL_HASHTABLE_H_

#include "memory.h"
#include "vector.h"
#include <utility>
#include <cstddef>

static const int __stl_num_primes = 28;

static const unsigned long __stl_prime_list[__stl_num_primes] =
    {
        53ul, 97ul, 193ul, 389ul, 769ul,
        1543ul, 3079ul, 6151ul, 12289ul, 24593ul,
        49157ul, 98317ul, 196613ul, 393241ul, 786433ul,
        1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul,
        50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul,
        1610612741ul, 3221225473ul, 4294967291ul};

// 以下找出上述 28 个质数之中，最接近并大于或等于 n 的那个质数
inline unsigned long __stl_next_prime(unsigned long n)
{
    const unsigned long *__first = __stl_prime_list;
    const unsigned long *__last = __stl_prime_list + (int)__stl_num_primes;
    const unsigned long *pos = lower_bound(__first, __last, n);
    return pos == __last ? *(__last - 1) : *pos;
}

namespace SimpleSTL
{
    template <class Val>
    struct __hashtable_node
    {
        __hashtable_node *next;
        Val val;
    };

    template <class Val, class Key, class HashFcn,
              class ExtractKey, class EqualKey, class Alloc = alloc2>
    class hashtable;

    template <class Val, class Key, class HashFcn,
              class ExtractKey, class EqualKey, class Alloc>
    struct __hashtable_iterator;

    template <class Val, class Key, class HashFcn,
              class ExtractKey, class EqualKey, class Alloc>
    struct __hashtable_const_iterator;

    template <class Val, class Key, class HashFcn,
              class ExtractKey, class EqualKey, class Alloc>
    struct __hashtable_iterator
    {
        // typedef hashtable<Val, Key, HashFcn, ExtractKey, EqualKey, Alloc>
        // hashtable;
        typedef __hashtable_iterator<Val, Key, HashFcn,
                                     ExtractKey, EqualKey, Alloc>
            iterator;
        typedef __hashtable_const_iterator<Val, Key, HashFcn,
                                           ExtractKey, EqualKey, Alloc>
            const_iterator;
        typedef __hashtable_node<Val> node;

        typedef forward_iterator_tag iterator_category;
        typedef Val value_type;
        typedef ptrdiff_t difference_type;
        typedef size_t size_type;
        typedef Val &reference;
        typedef Val *pointer;
        node *cur;
        hashtable<Val, Key, HashFcn, ExtractKey, EqualKey, Alloc> *ht;

        __hashtable_iterator(node *n, hashtable<Val, Key, HashFcn, ExtractKey, EqualKey, Alloc> *__tab)
            : cur(n), ht(__tab) {}
        __hashtable_iterator() {}
        reference operator*() const { return cur->val; }
        pointer operator->() const
        {
            return &(operator*());
        }
        iterator &operator++();
        iterator operator++(int);
        bool operator==(const iterator &__it) const
        {
            return cur == __it.cur;
        }
        bool operator!=(const iterator &__it) const
        {
            return cur != __it.cur;
        }
    };

    template <class Val, class Key, class HF, class ExK, class EqK,
              class ALL>
    __hashtable_iterator<Val, Key, HF, ExK, EqK, ALL> &
    __hashtable_iterator<Val, Key, HF, ExK, EqK, ALL>::operator++()
    {
        const node *old = cur;
        cur = cur->next;
        if (!cur)
        {
            size_type bucket = ht->bkt_num(old->val);
            while (!cur && ++bucket < ht->buckets.size())
                cur = ht->buckets[bucket];
        }
        return *this;
    }

    template <class Val, class Key, class HF, class ExK, class EqK,
              class ALL>
    inline __hashtable_iterator<Val, Key, HF, ExK, EqK, ALL>
    __hashtable_iterator<Val, Key, HF, ExK, EqK, ALL>::operator++(int)
    {
        iterator __tmp = *this;
        ++*this;
        return __tmp;
    }

    template <class Val, class Key, class HashFcn,
              class ExtractKey, class EqualKey, class Alloc>
    class hashtable
    {
    public:
        typedef Key key_type;       // 为 template 型别重新定义一个名称
        typedef Val value_type;     // 为 template 型别重新定义一个名称
        typedef HashFcn hasher;     // hash function 的函数型别
        typedef EqualKey key_equal; // 判断键值相同与否的方法

        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;

        typedef __hashtable_iterator<Val, Key, HashFcn, ExtractKey, EqualKey, Alloc>
            iterator;
        typedef __hashtable_const_iterator<Val, Key, HashFcn, ExtractKey, EqualKey, Alloc>
            const_iterator; // 这里只定义了 iterator，const_iterator 是类似的

        hasher hash_funct() const { return hash; }
        key_equal key_eq() const { return equals; }
        friend struct
            __hashtable_iterator<Val, Key, HashFcn, ExtractKey, EqualKey, Alloc>;
        friend struct
            __hashtable_const_iterator<Val, Key, HashFcn, ExtractKey, EqualKey, Alloc>;

    private:
        hasher hash;
        key_equal equals;
        ExtractKey get_key;

        typedef __hashtable_node<Val> node;
        typedef simple_alloc<node, Alloc> node_allocator;

        vector<node *, Alloc> buckets; // 以 vector 完成
        size_type num_elements;

    public:
        void initialize_buckets(size_type __n)
        {
            const size_type __n_buckets = next_size(__n);
            buckets.reserve(__n_buckets);
            buckets.insert(buckets.end(), __n_buckets, (node *)0);
            num_elements = 0;
        }

        void copy_from(const hashtable &__ht);

        void clear();

        void resize(size_type __num_elements_hint);

        size_type next_size(size_type __n) const
        {
            return __stl_next_prime(__n);
        }

    public:
        hashtable(size_type n, const HashFcn &hf, const EqualKey &eql)
            : hash(hf), equals(eql), get_key(ExtractKey()), num_elements(0)
        {
            initialize_buckets(n);
        }

        hashtable(const hashtable &__ht)
            : hash(__ht.hash),
              equals(__ht.equals),
              get_key(__ht.get_key),
              //   buckets(__ht.get_allocator()),
              num_elements(0)
        {
            copy_from(__ht);
        }

        hashtable &operator=(const hashtable &__ht)
        {
            if (&__ht != this)
            {
                clear();
                hash = __ht.hash;
                equals = __ht.equals;
                get_key = __ht.get_key;
                copy_from(__ht);
            }
            return *this;
        }

        ~hashtable() { clear(); }

        size_type bucket_count() const { return buckets.size(); }

        size_type max_bucket_count() const
        {
            return __stl_prime_list[(int)__stl_num_primes - 1];
        } // 其值将为 4294967291

        size_type elems_in_bucket(size_type __bucket)
        {
            size_type __result = 0;
            for (const node *cur = buckets[__bucket]; cur; cur = cur->next)
                __result += 1;
            return __result;
        }

        node *new_node(const value_type &obj)
        {
            node *n = node_allocator::allocate();
            n->next = 0;
            construct(&n->val, obj);
            return n;
        }

        void delete_node(node *n)
        {
            destroy(&n->val);
            node_allocator::deallocate(n);
        }

        iterator begin()
        {
            for (size_type __n = 0; __n < buckets.size(); ++__n)
                if (buckets[__n])
                    return iterator(buckets[__n], this);
            return end();
        }

        iterator end() { return iterator(0, this); }

        const_iterator begin() const
        {
            for (size_type __n = 0; __n < buckets.size(); ++__n)
                if (buckets[__n])
                    return const_iterator(buckets[__n], this);
            return end();
        }

        size_type size() const { return num_elements; }
        size_type max_size() const { return size_type(-1); }
        bool empty() const { return size() == 0; }

        const_iterator end() const { return const_iterator(0, this); }

        size_type count(const key_type &__key)
        {
            const size_type __n = bkt_num_key(__key);
            size_type __result = 0;

            for (const node *__cur = buckets[__n]; __cur; __cur = __cur->next)
                if (equals(get_key(__cur->val), __key))
                    ++__result;
            return __result;
        }

        iterator find(const key_type &__key)
        {
            size_type __n = bkt_num_key(__key);
            node *__first;
            for (__first = buckets[__n];
                 __first && !equals(get_key(__first->val), __key);
                 __first = __first->next)
            {
            }
            return iterator(__first, this);
        }

        const_iterator find(const key_type &__key) const
        {
            size_type __n = bkt_num_key(__key);
            const node *__first;
            for (__first = buckets[__n];
                 __first && !equals(get_key(__first->val), __key);
                 __first = __first->next)
            {
            }
            return const_iterator(__first, this);
        }

        reference find_or_insert(const value_type &__obj);
        pair<iterator, bool> insert_unique(const value_type &__obj)
        {
            resize(num_elements + 1);
            return insert_unique_noresize(__obj);
        }
        pair<iterator, bool> insert_unique_noresize(const value_type &__obj);

        iterator insert_equal(const value_type &__obj)
        {
            resize(num_elements + 1);
            return insert_equal_noresize(__obj);
        }
        iterator insert_equal_noresize(const value_type &__obj);

        // pair<iterator, iterator>
        // equal_range(const key_type &__key);

        // pair<const_iterator, const_iterator>
        // equal_range(const key_type &__key) const;

        size_type erase(const key_type &__key);
        void erase(const iterator &__it);
        void erase(iterator __first, iterator __last);

        // void erase(const const_iterator &__it);
        // void erase(const_iterator __first, const_iterator __last);

        void erase_bucket(const size_type __n, node *__first, node *__last);
        void erase_bucket(const size_type __n, node *__last);

        // 只接受键值
        size_type bkt_num_key(const key_type &__key) const
        {
            return bkt_num_key(__key, buckets.size());
        }

        // 只接受实值（value）
        size_type bkt_num(const value_type &__obj) const
        {
            return bkt_num_key(get_key(__obj));
        }

        // 接受键值和 buckets 个数
        size_type bkt_num_key(const key_type &__key, size_t __n) const
        {
            return hash(__key) % __n;
        }

        // 接受实值和 buckets 个数
        size_type bkt_num(const value_type &__obj, size_t __n) const
        {
            return bkt_num_key(get_key(__obj), __n);
        }
    };

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::clear()
    {
        // 针对每一个 bucket
        for (size_type i = 0; i < buckets.size(); ++i)
        {
            node *cur = buckets[i];
            // 将 bucket list 中的每一个节点删掉
            while (cur != 0)
            {
                node *__next = cur->next;
                delete_node(cur);
                cur = __next;
            }
            buckets[i] = 0;
        }
        num_elements = 0;
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::copy_from(const hashtable &__ht)
    {
        buckets.clear();
        buckets.reserve(__ht.buckets.size());
        buckets.insert(buckets.end(), __ht.buckets.size(), (node *)0);
        for (size_type i = 0; i < __ht.buckets.size(); ++i)
        {
            const node *__cur = __ht.buckets[i];
            if (__cur)
            {
                node *__copy = new_node(__cur->val);
                buckets[i] = __copy;

                for (node *__next = __cur->next;
                     __next;
                     __cur = __next, __next = __cur->next)
                {
                    __copy->next = new_node(__next->val);
                    __copy = __copy->next;
                }
            }
        }
        num_elements = __ht.num_elements;
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::resize(size_type __num_elements_hint)
    {
        const size_type __old_n = buckets.size();
        if (__num_elements_hint > __old_n)
        {
            const size_type __n = next_size(__num_elements_hint);
            if (__n > __old_n)
            {
                vector<node *, _All> __tmp(__n, (node *)(0)); // 设立新的 bucket
                // 以下处理每一个旧的 bucket
                for (size_type __bucket = 0; __bucket < __old_n; ++__bucket)
                {
                    node *__first = buckets[__bucket];
                    // 以下处理每一个旧 bucker 所含（串行）的节点
                    while (__first)
                    {
                        size_type __new_bucket = bkt_num(__first->val, __n);
                        // (1) 令旧 bucket 指向其所对应之串行的下一个节点（以便迭代处理）
                        buckets[__bucket] = __first->next;
                        // (2)(3) 将当前节点插入到新 bucket 内，成为其对应串行的第一个节点
                        __first->next = __tmp[__new_bucket];
                        __tmp[__new_bucket] = __first;
                        // (4) 回到旧 bucket 所指的待处理串行，准备处理下一个节点
                        __first = buckets[__bucket];
                    }
                }
                buckets.swap(__tmp); // 新旧两个 bucket 对换指针
                // 注意，对调双方如果大小不同，大的会变小，小的会变大
                // 离开时释放 local tmp 的内存
            }
        }
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    std::pair<typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::iterator, bool>
    hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::insert_unique_noresize(const value_type &__obj)
    {
        const size_type __n = bkt_num(__obj);
        node *__first = buckets[__n];

        for (node *__cur = __first; __cur; __cur = __cur->next)
            if (equals(get_key(__cur->val), get_key(__obj)))
                // 如果发现与链表中的某键值相同，就不插入，立刻返回
                return pair<iterator, bool>(iterator(__cur, this), false);

        node *__tmp = new_node(__obj);
        __tmp->next = __first;
        buckets[__n] = __tmp;
        ++num_elements;
        return pair<iterator, bool>(iterator(__tmp, this), true);
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::iterator
    hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::insert_equal_noresize(const value_type &__obj)
    {
        const size_type __n = bkt_num(__obj);
        node *__first = buckets[__n];

        for (node *__cur = __first; __cur; __cur = __cur->next)
            // 如果发现与链表中的某键值相同，就马上插入，然后返回
            if (equals(get_key(__cur->val), get_key(__obj)))
            {
                node *__tmp = new_node(__obj);
                __tmp->next = __cur->next;
                __cur->next = __tmp;
                ++num_elements;
                return iterator(__tmp, this);
            }

        node *__tmp = new_node(__obj); // 产生新节点
        __tmp->next = __first;         // 将新节点插入至链表头部
        buckets[__n] = __tmp;
        ++num_elements;
        return iterator(__tmp, this);
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::size_type
    hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase(const key_type &__key)
    {
        const size_type __n = bkt_num_key(__key);
        node *__first = buckets[__n];
        size_type __erased = 0;

        if (__first)
        {
            node *__cur = __first;
            node *__next = __cur->next;
            while (__next)
            {
                if (equals(get_key(__next->val), __key))
                {
                    __cur->next = __next->next;
                    delete_node(__next);
                    __next = __cur->next;
                    ++__erased;
                    --num_elements;
                }
                else
                {
                    __cur = __next;
                    __next = __cur->next;
                }
            }
            if (equals(get_key(__first->val), __key))
            {
                buckets[__n] = __first->next;
                delete_node(__first);
                ++__erased;
                --num_elements;
            }
        }
        return __erased;
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase(const iterator &__it)
    {
        node *__p = __it.cur;
        if (__p)
        {
            const size_type __n = bkt_num(__p->val);
            node *__cur = buckets[__n];

            if (__cur == __p)
            {
                buckets[__n] = __cur->next;
                delete_node(__cur);
                --num_elements;
            }
            else
            {
                node *__next = __cur->next;
                while (__next)
                {
                    if (__next == __p)
                    {
                        __cur->next = __next->next;
                        delete_node(__next);
                        --num_elements;
                        break;
                    }
                    else
                    {
                        __cur = __next;
                        __next = __cur->next;
                    }
                }
            }
        }
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase(iterator __first, iterator __last)
    {
        size_type __f_bucket = __first.cur ? bkt_num(__first.cur->val) : buckets.size();
        size_type __l_bucket = __last.cur ? bkt_num(__last.cur->val) : buckets.size();

        if (__first.cur == __last.cur)
            return;
        else if (__f_bucket == __l_bucket)
            erase_bucket(__f_bucket, __first.cur, __last.cur);
        else
        {
            erase_bucket(__f_bucket, __first.cur, 0);
            for (size_type __n = __f_bucket + 1; __n < __l_bucket; ++__n)
                erase_bucket(__n, 0);
            if (__l_bucket != buckets.size())
                erase_bucket(__l_bucket, __last.cur);
        }
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase_bucket(const size_type __n, node *__first, node *__last)
    {
        node *__cur = buckets[__n];
        if (__cur == __first)
            erase_bucket(__n, __last);
        else
        {
            node *__next;
            for (__next = __cur->next;
                 __next != __first;
                 __cur = __next, __next = __cur->next)
                ;
            while (__next != __last)
            {
                __cur->next = __next->next;
                delete_node(__next);
                __next = __cur->next;
                --num_elements;
            }
        }
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase_bucket(const size_type __n, node *__last)
    {
        node *__cur = buckets[__n];
        while (__cur != __last)
        {
            node *__next = __cur->next;
            delete_node(__cur);
            __cur = __next;
            buckets[__n] = __cur;
            --num_elements;
        }
    }

    template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
    typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::reference
    hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::find_or_insert(const value_type &__obj)
    {
        resize(num_elements + 1);

        size_type __n = bkt_num(__obj);
        node *__first = buckets[__n];

        for (node *__cur = __first; __cur; __cur = __cur->next)
            if (equals(get_key(__cur->val), get_key(__obj)))
                return __cur->val;

        node *__tmp = new_node(__obj);
        __tmp->next = __first;
        buckets[__n] = __tmp;
        ++num_elements;
        return __tmp->val;
    }
}

#endif