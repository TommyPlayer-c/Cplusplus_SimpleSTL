#ifndef _SIMPLE_STL_RBTREE_H_
#define _SIMPLE_STL_RBTREE_H_

#include "./stl_iterator.h"
#include "./memory.h"
#include <cstddef>
#include <utility>

namespace SimpleSTL
{
    typedef bool __rb_tree_color_type;
    const __rb_tree_color_type __rb_tree_red = false;
    const __rb_tree_color_type __rb_tree_black = true;

    struct __rb_tree_node_base
    {
        typedef __rb_tree_color_type color_type;
        typedef __rb_tree_node_base *base_ptr;

        color_type color;
        base_ptr parent; // RBtree的许多操作，必须知道父节点
        base_ptr left;
        base_ptr right;

        static base_ptr minimum(base_ptr x)
        {
            while (x->left != 0)
                x = x->left; // 一直向左走，就会找到最小值，这是二叉搜索树的特性
            return x;
        }

        static base_ptr maximum(base_ptr x)
        {
            while (x->right != 0)
                x = x->right; // 一直向右走，就会找到最大值，这是二叉搜索树的特性
            return x;
        }
    };

    template <class Value>
    struct __rb_tree_node : public __rb_tree_node_base
    {
        typedef __rb_tree_node<Value> *link_type;
        Value value_field; // 节点值
    };

    // 基层迭代器
    struct __rb_tree_base_iterator
    {
        typedef __rb_tree_node_base::base_ptr base_ptr;
        typedef bidirectional_iterator_tag iterator_category;
        typedef ptrdiff_t difference_type;

        base_ptr node; // 它用来于容器产生一个连结关系（make a reference）

        // 以下其实可实现于 operator++ 内，因为再无他处会调用此函数了（P216）
        void increment()
        {
            if (node->right != 0)
            {
                node = node->right;
                while (node->left != 0)
                    node = node->left;
            }
            else
            {
                base_ptr y = node->parent; // 找出父节点
                while (node == y->right)   // 如果现行节点本身是个右子节点
                {
                    node = y; // 就一直上溯，直到 “不为右子节点” 为止
                    y = y->parent;
                }
                if (node->right != y)
                    node = y;
            }
        }

        // 以下其实可实现于 operator-- 内，因为再无他处会调用此函数了
        void decrement()
        {
            if (node->color == __rb_tree_red &&
                node->parent->parent == node)
                node = node->right;
            else if (node->left != 0)
            {
                base_ptr y = node->left;
                while (y->right != 0)
                    y = y->right;
                node = y;
            }
            else
            {
                base_ptr y = node->parent;
                while (node == y->left)
                {
                    node = y;
                    y = y->parent;
                }
                node = y;
            }
        }
    };

    // RB-tree的正规迭代器
    template <class Value, class Ref, class Ptr>
    struct __rb_tree_iterator : public __rb_tree_base_iterator
    {
        typedef Value value_type;
        typedef Ref reference;
        typedef Ptr pointer;
        typedef __rb_tree_iterator<Value, Value &, Value *> iterator;
        typedef __rb_tree_iterator<Value, const Value &, const Value *> const_iterator;
        typedef __rb_tree_iterator<Value, Ref, Ptr> self;
        typedef __rb_tree_node<Value> *link_type;

        __rb_tree_iterator() {}
        __rb_tree_iterator(link_type x) { node = x; }
        __rb_tree_iterator(const iterator &it) { node = it.node; }

        reference operator*() const { return link_type(node)->value_field; }
        pointer operator->() const { return &(operator*()); }

        self &operator++()
        {
            increment();
            return *this;
        }
        self operator++(int)
        {
            self tmp = *this;
            increment();
            return tmp;
        }

        self &operator--()
        {
            decrement();
            return *this;
        }
        self operator--(int)
        {
            self tmp = *this;
            decrement();
            return tmp;
        }

        bool operator==(const self &iter) const { return node == iter.node; }
        bool operator!=(const self &iter) const { return node != iter.node; }
    };

    template <class Key, class Value, class KeyOfValue, class Compare,
              class Alloc = alloc2>
    class rb_tree
    {
    protected:
        typedef void *void_pointer;
        typedef __rb_tree_node_base *base_ptr;
        typedef __rb_tree_node<Value> rb_tree_node;
        typedef simple_alloc<rb_tree_node, Alloc> rb_tree_node_allocator;
        typedef __rb_tree_color_type color_type;

    public:
        typedef Key key_type;
        typedef Value value_type;

        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;

        typedef rb_tree_node *link_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

    protected:
        link_type get_node() { return rb_tree_node_allocator::allocate(); }
        void put_node(link_type p) { rb_tree_node_allocator::deallocate(p); }
        // void destroy(Value *value_field) {}
        link_type create_node(const value_type &x)
        {
            link_type tmp = get_node();         // 配置空间
            construct(&tmp->value_field, x);    // 构造内容
            return tmp;
        }

        link_type clone_node(link_type x)       // 复制一个节点（的值和色）
        {
            link_type tmp = create_node(x->value_field);
            tmp->color = x->color;
            tmp->left = 0;
            tmp->right = 0;
            return tmp;
        }

        void destroy_node(link_type p)
        {
            destroy(&p->value_field);       // 析构内容
            put_node(p);                    // 释放内存
        }

        void clear() {
            if (node_count != 0) {
                erase(root());
                leftmost() = header;
                root() = 0;
                rightmost() = header;
                node_count = 0;
            }
        }

    protected:
        // RB-tree 只以三笔数据表现
        size_type node_count;   // 追踪记录树的大小（节点数量）
        link_type header;       // 这是实现上的一个小技巧，header与root互为父节点
        Compare key_compare;    // 节点间的键值大小比较准则，应该会是个 function object

        // 以下三个函数用来方便取得header的成员
        link_type& root() const { return (link_type&)header->parent; }
        link_type& leftmost() const { return (link_type&)header->left; }
        link_type& rightmost() const { return (link_type&)header->right; }

        // 以下六个函数用来方便取得节点 x 的成员
        static link_type &left(link_type x)
            { return (link_type &)(x->left); }
        static link_type &right(link_type x)
            { return (link_type &)(x->right); }
        static link_type &parent(link_type x)
            { return (link_type &)(x->parent); }
        static reference value(link_type x)
            { return x->value_field; }
        static const Key &key(link_type x)
            { return KeyOfValue()(value(x)); }
        static color_type &color(link_type x)
            { return (color_type &)(x->color); }

        // 以下六个函数用来方便取得x的成员
        static link_type &left(base_ptr x)
            { return (link_type &)(x->left); }
        static link_type &right(base_ptr x)
            { return (link_type &)(x->right); }
        static link_type &parent(base_ptr x)
            { return (link_type &)(x->parent); }
        static reference value(base_ptr x)
            { return ((link_type)x)->value_field; }
        static const Key &key(base_ptr x)
            { return KeyOfValue()(value(link_type(x))); }
        static color_type &color(base_ptr x)
            { return (color_type &)(link_type(x)->color); }
        
        static link_type minimum(link_type x)
        {
            return (link_type)__rb_tree_node_base::minimum(x);
        }

        static link_type maximum(link_type x)
        {
            return (link_type)__rb_tree_node_base::maximum(x);
        }

    public:
        typedef __rb_tree_iterator<value_type, reference, pointer> iterator;
        typedef __rb_tree_iterator<value_type, const_reference, const_pointer> const_iterator;

    private:
        // 真正的插入执行程序
        iterator __insert(base_ptr x_, base_ptr y_, const Value &v);
        link_type __copy(link_type x, link_type p);
        void __erase(link_type x);
        void init()
        {
            header = get_node();            // 产生一个节点空间，令 header 指向它
            color(header) = __rb_tree_red;  // 令 header 为红色，用来区分 header 与 root

            root() = 0;
            leftmost() = header;        // 令 header 的左子节点为自己
            rightmost() = header;       // 令 header 的右子节点为自己
        }

    public:
        rb_tree(const Compare &comp = Compare())
            : node_count(0), key_compare(comp) { init(); }

        ~rb_tree()
        {
            clear();
            put_node(header);
        }
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc> &
            operator=(const rb_tree<Key, Value, KeyOfValue, Compare, Alloc> &x);
        link_type _M_copy(link_type __x, link_type __p);

    public:
        Compare key_comp() const { return key_compare; }
        iterator begin() const { return leftmost(); }
        iterator end() const { return header; }
        bool empty() const { return node_count == 0; }
        size_type size() const { return node_count; }
        size_type max_size() const { return size_type(-1); }
        void swap(rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& t) {
            std::swap(header, t.header);
            std::swap(node_count, t.node_count);
            std::swap(key_compare, t.key_compare);
        }

    public:
        // 将 x 插入到 RB-tree 中（保持节点值独一无二）
        iterator insert_equal(const value_type &v);
        
        // 将 x 插入到 RB-tree 中（保持节点值独一无二）
        std::pair<iterator, bool> insert_unique(const value_type &v);
        iterator insert_unique(iterator position, const value_type& x);
        template <class InputIterator>
        bool insert_unique(InputIterator first, InputIterator last)
        {
            for (auto it = first; it != last; it++)
            {
                insert_unique(*it);
            }
        }

        void erase(iterator position);
        size_type erase(const key_type& x);
        void erase(iterator first, iterator last);
        void erase(const key_type* first, const key_type* last);

        iterator find(const key_type& __x);
        const_iterator find(const key_type& __x) const;
        size_type count(const key_type& __x) const;
        iterator lower_bound(const key_type& __x);
        const_iterator lower_bound(const key_type& __x) const;
        iterator upper_bound(const key_type& __x);
        const_iterator upper_bound(const key_type& __x) const;
        std::pair<iterator,iterator> equal_range(const key_type& __x);
        std::pair<const_iterator, const_iterator> equal_range(const key_type& __x) const;
    };

    template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
    typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator 
    rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_equal(const value_type &v)
    {
        link_type y = header;
        link_type x = root();   // 从根节点开始
        while (x != 0)          // 从根节点开始，往下寻找适当的插入点
        {
            y = x;
            x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x);
            // 以上，遇“大”则往左，遇“小于或等于”则往右
        }
        return __insert(x, y, v);
        // 以上，x 为新值插入点，y 为插入点之父节点，v 为新值
    }

    template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
    std::pair<typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator, bool>
    rb_tree<Key, Value, KeyOfValue, Compare, Alloc>:: 
        insert_unique(const value_type &v)
    {
        link_type y = header;
        link_type x = root();
        bool comp = true;
        while (x != 0)
        {
            y = x;
            comp = key_compare(KeyOfValue()(v), key(x));
            x = comp ? left(x) : right(x);
        }
        //离开 while 循环之后，y 所指即插入点之父节点（此时的它必为叶节点），x 必定为 NULL
        
        iterator j = iterator(y);
        if (comp)   // comp 为真，表示遇“大”，将插入于左侧
            if (j == begin())   // 如果插入点之父节点为最左节点
                return pair<iterator, bool>(__insert(x, y, v), true);
                // 以上，x 为新值插入点，y 为插入点之父节点，v 为新值
            else
                --j;    // 调整 j，回头准备测试
        if (key_compare(key(j.node), KeyOfValue()(v)))
            // 新键值不与既有节点之键值重复，于是以下执行安插操作
            return pair<iterator, bool>(__insert(x, y, v), true);
        
        // 进行至此，表示新值一定与树中键值重复，那么就不该插入该值
        return pair<iterator, bool>(j, false);
    }

    // 侯捷代码中没有，从 SGI_STL 中改写
    template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
    typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator 
    rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::
        insert_unique(iterator position, const value_type& v)
    {
        if (position.node == header->left) { // begin()
            if (size() > 0 && 
                key_compare(KeyOfValue()(v), Key(position.node)))
            return __insert(position.node, position.node, v);
            // first argument just needs to be non-null 
            else
                return insert_unique(v).first;
        } 
        else if (position.node == header) { // end()
            if (key_compare(Key(rightmost()), KeyOfValue()(v)))
                return __insert(0, rightmost(), v);
            else
                return insert_unique(v).first;
        } 
        else {
            iterator __before = position;
            --__before;
            if (key_compare(Key(__before.node), KeyOfValue()(v)) 
              && key_compare(KeyOfValue()(v), Key(position.node))) {
            if (right(__before.node) == 0)
                return __insert(0, __before.node, v); 
            else
                return __insert(position.node, position.node, v);
            // first argument just needs to be non-null 
            } 
            else
                return insert_unique(v).first;
        }        
    }

    template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
    typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator
    rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::
        __insert(base_ptr x_, base_ptr y_, const Value &v)
    {   // 参数 x_ 为新值插入点，参数 y_ 为插入点之父节点，参数 v 为新值
        link_type x = (link_type) x_;
        link_type y = (link_type) y_;
        link_type z;
        
        if (y == header || x != 0 || key_compare(KeyOfValue()(v), key(y)))
        {
            z = create_node(v); // 产生一个新节点
            left(y) = z;        // 这使得 y 即为 header 时，leftmost() = z
            if (y == header)
            {
                root() = z;
                rightmost() = z;
            }
            else if (y == leftmost())   // 如果 y 为最左节点
                leftmost() = z;
        }
        else
        {
            z = create_node(v);
            right(y) = z;
            if (y == rightmost())
                rightmost() = z;
        }
        parent(z) = y;
        left(z) = 0;
        right(z) = 0;
        
        __rb_tree_rebalance(z, header->parent);
        ++node_count;
        return iterator(z);     // 返回一个迭代器，指向新增节点
    }

    template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::size_type 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::erase(const _Key& __x)
    {
      pair<iterator,iterator> __p = equal_range(__x);
      size_type __n = 0;
      distance(__p.first, __p.second, __n);
      erase(__p.first, __p.second);
      return __n;
    }

    template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
    inline void rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
    ::erase(iterator __position)
    {
        link_type __y = 
          (link_type) __rb_tree_rebalance_for_erase(__position.node,
                                                    header->parent,
                                                    header->left,
                                                    header->right);
        destroy_node(__y);
        --node_count;
    }


    template <class _Key, class _Value, class _KeyOfValue, 
              class _Compare, class _Alloc>
    void rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::erase(iterator __first, iterator __last)
    {
      if (__first == begin() && __last == end())
        clear();
      else
        while (__first != __last) erase(__first++);
    }

    template <class _Key, class _Value, class _KeyOfValue, 
              class _Compare, class _Alloc>
    void rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::erase(const _Key* __first, const _Key* __last) 
    {
      while (__first != __last) erase(*__first++);
    }

        // 全局函数
        // 新节点必为红节点。如果插入处之父节点亦为红节点，就违反红黑树规则
        // 此时可能需做树形旋转及颜色改变
        inline void
        __rb_tree_rotate_left(__rb_tree_node_base* x,
                              __rb_tree_node_base*& root)
        {
            // x 为旋转点
            __rb_tree_node_base *y = x->right;
            x->right = y->left;
            if (y->left != 0)
                y->left->parent = x;    // 别忘了回马枪设定父节点
            y->parent = x->parent;

            // 令 y 完全顶替 x 的地位（必须将 x 对其父节点的关系完全接收过来）
            if (x == root)
                root = y;
            else if (x == x->parent->left)
                x->parent->left = y;
            else
                x->parent->right = y;
            y->left = x;
            x->parent = y;
        }

    // 全局函数
    // 新节点必为红节点。如果插入处之父节点亦为红节点，就违反红黑树规则
    // 此时可能需做树形旋转及颜色改变
    inline void
    __rb_tree_rotate_right(__rb_tree_node_base *x,
                           __rb_tree_node_base *&root)
    {
        //x为旋转点
        __rb_tree_node_base *y = x->left;
        x->left = y->right;
        if (y->right != 0)
            y->right->parent = x;
        y->parent = x->parent;
        
        // 令 y 完全顶替 x 的地位（必须将 x 对其父节点的关系完全接收过来）
        if (x == root)
            root = y;
        else if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    // 全局函数
    // 重新令树形平衡（改变颜色及旋转树形）
    // 参数 1 为新增节点，参数 2 为 root
    inline void __rb_tree_rebalance(__rb_tree_node_base* x, __rb_tree_node_base*& root)
    {
        x->color = __rb_tree_red;           // 新节点必为红色
        while (x != root && x->parent->color == __rb_tree_red)  // 父节点为红
        {
            if (x->parent == x->parent->parent->left)   // 父节点为祖父节点之左子节点
            { 
                __rb_tree_node_base *y = x->parent->parent->right; //令 y 为伯父节点
                if (y && y->color == __rb_tree_red)             //伯父节点存在，且为红
                {                                             
                    x->parent->color = __rb_tree_black;         // 更改父节点为黑
                    y->color = __rb_tree_black;                 // 更改伯父节点为黑
                    x->parent->parent->color = __rb_tree_red;   // 更改祖父节点为红
                    x = x->parent->parent;                      // 准备继续往上层检查
                }
                else    // 无伯父节点，或伯父节点为黑
                { 
                    if (x == x->parent->right)  //如果新节点为父节点之右子节点
                    {
                        x = x->parent;
                        __rb_tree_rotate_left(x, root);     // 第一参数为左旋点
                    }
                    x->parent->color = __rb_tree_black;
                    x->parent->parent->color = __rb_tree_red;
                    __rb_tree_rotate_right(x->parent->parent, root); // 第一参数为右旋点
                }
            }
            else    // 父节点为祖父节点之右子节点
            {                                                     
                __rb_tree_node_base *y = x->parent->parent->left; // 令 y 为伯父节点
                if (y && y->color == __rb_tree_red)
                {                                               // 有伯父节点，且为红
                    x->parent->color = __rb_tree_black;         // 更改父节点为黑
                    y->color = __rb_tree_black;                 // 更改伯父节点为黑
                    x->parent->parent->color = __rb_tree_red;   // 更改祖父节点为红
                    x = x->parent->parent;                      // 准备继续往上层检查
                }
                else     // 无伯父节点，或伯父节点为黑
                {
                    if (x == x->parent->left)   // 如果新节点为父节点之左子节点
                    { 
                        x = x->parent;
                        __rb_tree_rotate_right(x, root);    // 第一参数为右旋点
                    }
                    x->parent->color = __rb_tree_black;
                    x->parent->parent->color = __rb_tree_red;
                    __rb_tree_rotate_left(x->parent->parent, root); //第一参数为左旋点
                }
            }
        }   // while 结束
        
        root->color = __rb_tree_black;  // 根节点永远为黑
    }

    inline __rb_tree_node_base*
    __rb_tree_rebalance_for_erase(__rb_tree_node_base* __z,
                                 __rb_tree_node_base*& __root,
                                 __rb_tree_node_base*& __leftmost,
                                 __rb_tree_node_base*& __rightmost)
    {
      __rb_tree_node_base* __y = __z;
      __rb_tree_node_base* __x = 0;
      __rb_tree_node_base* __x_parent = 0;
      if (__y->left == 0)     // __z has at most one non-null child. y == z.
        __x = __y->right;     // __x might be null.
      else
        if (__y->right == 0)  // __z has exactly one non-null child. y == z.
          __x = __y->left;    // __x is not null.
        else {                   // __z has two non-null children.  Set __y to
          __y = __y->right;   //   __z's successor.  __x might be null.
          while (__y->left != 0)
            __y = __y->left;
          __x = __y->right;
        }
      if (__y != __z) {          // relink y in place of z.  y is z's successor
        __z->left->parent = __y; 
        __y->left = __z->left;
        if (__y != __z->right) {
          __x_parent = __y->parent;
          if (__x) __x->parent = __y->parent;
          __y->parent->left = __x;      // __y must be a child of left
          __y->right = __z->right;
          __z->right->parent = __y;
        }
        else
          __x_parent = __y;  
        if (__root == __z)
          __root = __y;
        else if (__z->parent->left == __z)
          __z->parent->left = __y;
        else 
          __z->parent->right = __y;
        __y->parent = __z->parent;
        std::swap(__y->color, __z->color);
        __y = __z;
        // __y now points to node to be actually deleted
      }
      else {                        // __y == __z
        __x_parent = __y->parent;
        if (__x) __x->parent = __y->parent;   
        if (__root == __z)
          __root = __x;
        else 
          if (__z->parent->left == __z)
            __z->parent->left = __x;
          else
            __z->parent->right = __x;
        if (__leftmost == __z) 
          if (__z->right == 0)        // __z->left must be null also
            __leftmost = __z->parent;
        // makes __leftmost == header if __z == __root
          else
            __leftmost = __rb_tree_node_base::minimum(__x);
        if (__rightmost == __z)  
          if (__z->left == 0)         // __z->right must be null also
            __rightmost = __z->parent;  
        // makes __rightmost == header if __z == __root
          else                      // __x == __z->left
            __rightmost = __rb_tree_node_base::maximum(__x);
      }
      if (__y->color != __rb_tree_red) { 
        while (__x != __root && (__x == 0 || __x->color == __rb_tree_black))
          if (__x == __x_parent->left) {
            __rb_tree_node_base* __w = __x_parent->right;
            if (__w->color == __rb_tree_red) {
              __w->color = __rb_tree_black;
              __x_parent->color = __rb_tree_red;
              __rb_tree_rotate_left(__x_parent, __root);
              __w = __x_parent->right;
            }
            if ((__w->left == 0 || 
                 __w->left->color == __rb_tree_black) &&
                (__w->right == 0 || 
                 __w->right->color == __rb_tree_black)) {
              __w->color = __rb_tree_red;
              __x = __x_parent;
              __x_parent = __x_parent->parent;
            } else {
              if (__w->right == 0 || 
                  __w->right->color == __rb_tree_black) {
                if (__w->left) __w->left->color = __rb_tree_black;
                __w->color = __rb_tree_red;
                __rb_tree_rotate_right(__w, __root);
                __w = __x_parent->right;
              }
              __w->color = __x_parent->color;
              __x_parent->color = __rb_tree_black;
              if (__w->right) __w->right->color = __rb_tree_black;
              __rb_tree_rotate_left(__x_parent, __root);
              break;
            }
          } else {                  // same as above, with right <-> left.
            __rb_tree_node_base* __w = __x_parent->left;
            if (__w->color == __rb_tree_red) {
              __w->color = __rb_tree_black;
              __x_parent->color = __rb_tree_red;
              __rb_tree_rotate_right(__x_parent, __root);
              __w = __x_parent->left;
            }
            if ((__w->right == 0 || 
                 __w->right->color == __rb_tree_black) &&
                (__w->left == 0 || 
                 __w->left->color == __rb_tree_black)) {
              __w->color = __rb_tree_red;
              __x = __x_parent;
              __x_parent = __x_parent->parent;
            } else {
              if (__w->left == 0 || 
                  __w->left->color == __rb_tree_black) {
                if (__w->right) __w->right->color = __rb_tree_black;
                __w->color = __rb_tree_red;
                __rb_tree_rotate_left(__w, __root);
                __w = __x_parent->left;
              }
              __w->color = __x_parent->color;
              __x_parent->color = __rb_tree_black;
              if (__w->left) __w->left->color = __rb_tree_black;
              __rb_tree_rotate_right(__x_parent, __root);
              break;
            }
          }
        if (__x) __x->color = __rb_tree_black;
      }
      return __y;
    }

    template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::operator=(const rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x)
    {
        if (this != &__x) {
            // Note that _Key may be a constant type.
            clear();
            node_count = 0;
            key_compare = __x.key_compare;        
            if (__x.root() == 0) {
                root() = 0;
                leftmost() = header;
                rightmost() = header;
            }
            else {
                root() = _M_copy(__x.root(), header);
                leftmost() = minimum(root());
                rightmost() = maximum(root());
                node_count = __x.node_count;
            }
        }
        return *this;
    }

    template <class _Key, class _Val, class _KoV, class _Compare, class _Alloc>
    typename rb_tree<_Key, _Val, _KoV, _Compare, _Alloc>::link_type 
    rb_tree<_Key,_Val,_KoV,_Compare,_Alloc>
      ::_M_copy(link_type __x, link_type __p)
    {
                            // structural copy.  __x and __p must be non-null.
      link_type __top = clone_node(__x);
      __top->_M_parent = __p;
    
        if (__x->_M_right)
          __top->_M_right = _M_copy(right(__x), __top);
        __p = __top;
        __x = left(__x);
    
        while (__x != 0) {
          link_type __y = clone_node(__x);
          __p->_M_left = __y;
          __y->_M_parent = __p;
          if (__x->_M_right)
            __y->_M_right = _M_copy(right(__x), __y);
          __p = __y;
          __x = left(__x);
        }
      return __top;
    }

    template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::find(const key_type &k)
    {
        link_type y = header;   // last node which is not less than k
        link_type x = root();   // current node
        while (x != 0)
            if (!key_compare(key(x), k))
                // 进行到这里，表示 x 键值大于 k。遇到大值就向左走。
                y = x, x = left(x);      // 注意语法！
            else
                // 进行到这里，表示 x 键值小于 k。遇到大值就向左走。
                x = right(x);
        
        iterator j = iterator(y);
        return (j == end() || key_compare(k, key(j.node))) ? end() : j;
    }

    template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::const_iterator 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::find(const key_type &k) const
    {
        link_type y = header;   // last node which is not less than k
        link_type x = root();   // current node
        while (x != 0)
            if (!key_compare(key(x), k))
                // 进行到这里，表示 x 键值大于 k。遇到大值就向左走。
                y = x, x = left(x);      // 注意语法！
            else
                // 进行到这里，表示 x 键值小于 k。遇到大值就向左走。
                x = right(x);
        
        iterator j = iterator(y);
        return (j == end() || key_compare(k, key(j.node))) ? end() : j;
    }

    template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::size_type 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::count(const key_type& __k) const
    {
        pair<const_iterator, const_iterator> __p = equal_range(__k);
        size_type __n = 0;
        distance(__p.first, __p.second, __n);
        return __n;
    }

    template <class _Key, class _Value, class _KeyOfValue, 
              class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::lower_bound(const _Key& __k)
    {
      link_type __y = header; /* Last node which is not less than __k. */
      link_type __x = root(); /* Current node. */

      while (__x != 0) 
        if (!key_compare(key(__x), __k))
          __y = __x, __x = left(__x);
        else
          __x = right(__x);

      return iterator(__y);
    }

    template <class _Key, class _Value, class _KeyOfValue, 
              class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::const_iterator 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::lower_bound(const _Key& __k) const
    {
      link_type __y = header; /* Last node which is not less than __k. */
      link_type __x = root(); /* Current node. */

      while (__x != 0) 
        if (!key_compare(key(__x), __k))
          __y = __x, __x = left(__x);
        else
          __x = right(__x);

      return const_iterator(__y);
    }

    template <class _Key, class _Value, class _KeyOfValue, 
              class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::upper_bound(const _Key& __k)
    {
      link_type __y = header; /* Last node which is greater than __k. */
      link_type __x = root(); /* Current node. */

       while (__x != 0) 
         if (key_compare(__k, key(__x)))
           __y = __x, __x = left(__x);
         else
           __x = right(__x);

       return iterator(__y);
    }

    template <class _Key, class _Value, class _KeyOfValue, 
              class _Compare, class _Alloc>
    typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::const_iterator 
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::upper_bound(const _Key& __k) const
    {
      link_type __y = header; /* Last node which is greater than __k. */
      link_type __x = root(); /* Current node. */

       while (__x != 0) 
         if (key_compare(__k, key(__x)))
           __y = __x, __x = left(__x);
         else
           __x = right(__x);

       return const_iterator(__y);
    }

    template <class _Key, class _Value, class _KeyOfValue, 
              class _Compare, class _Alloc>
    inline 
    std::pair<typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator,
         typename rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>::iterator>
    rb_tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>
      ::equal_range(const _Key& __k)
    {
      return pair<iterator, iterator>(lower_bound(__k), upper_bound(__k));
    }

    template <class _Key, class _Value, class _KoV, class _Compare, class _Alloc>
    inline 
    std::pair<typename rb_tree<_Key, _Value, _KoV, _Compare, _Alloc>::const_iterator,
         typename rb_tree<_Key, _Value, _KoV, _Compare, _Alloc>::const_iterator >
    rb_tree<_Key, _Value, _KoV, _Compare, _Alloc>
      ::equal_range(const _Key& __k) const
    {
      return pair<const_iterator,const_iterator>(lower_bound(__k),
                                                 upper_bound(__k));
    }
}
#endif