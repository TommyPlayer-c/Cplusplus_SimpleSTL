#ifndef _SIMPLE_STL_LIST_H_
#define _SIMPLE_STL_LIST_H_

#include <stddef.h>
#include <initializer_list>
#include "./memory.h"
#include "./stl_iterator.h"

namespace SimpleSTL {
    template <class T>
    struct __list_node {  //list是双向的
    	typedef __list_node<T>* __list_node_pointer;
    	__list_node_pointer prev;
    	__list_node_pointer next;
    	T data;
    };
    
    // 除了vector和array，其他容器的迭代器均为class
    template<class T>
    struct __list_iterator {
        typedef __list_iterator<T> iterator; //指向内部元素值得迭代器
        typedef __list_iterator<T> self; //指向list节点的迭代器
        typedef bidirectional_iterator_tag iterator_category;
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef __list_node<T>* link_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        link_type node; // 迭代器内部的一个普通指针，指向list的节点

        // constructor
        __list_iterator(link_type x) : node(x) {}
        __list_iterator() {}
        __list_iterator(const iterator& x) : node(x.node) {}

        bool operator==(const self& x) const {return node == x.node;}
        bool operator!=(const self& x) const {return node != x.node;}
        
        // 以下对迭代器取值（deference），取的是节点的数据值。
        reference operator*() const { return (*node).data; }

        // 以下是迭代器的成员存取（member access）运算子的标准做法
        pointer operator->() const { return &(operator*()); }
        
        // ++i（prefix form）
        self& operator++() {
            node = node->next;
            return *this;
        }
        
        // i++（postfix form）
        self operator++(int) {
            self tmp = *this;   // 调用copy ctor
        	++*this;            // 调用 prefix form
            return tmp;
        }

        // --i（prefix form）
        self& operator--() {
        	node = node->prev;
        	return *this;
        }

        // --i（postfix form）
        self operator--(int) {
        	self tmp = *this;   // 调用copy ctor
        	--*this;            // 调用 prefix form
            return tmp;
        }

        self operator+(int dist){
            self tmp = *this;
            while (dist-- > 0) {
                tmp = tmp.node->next;
            }
            return tmp;
        }

        self operator-(int dist){
            self tmp = *this;
            while (dist-- > 0) {
                tmp = tmp.node->prev;
            }
            return tmp;
        }
    };

    template <class T, class Alloc = alloc2>
	class list {
	protected:
        typedef __list_node<T> list_node;
        typedef simple_alloc<list_node, Alloc> list_node_allocator;
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef value_type& reference;
        typedef value_type& const_reference;
        
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef list_node* link_type;
        typedef __list_iterator<T> iterator;

	    // 配置一个节点并传回
	    link_type get_node() {
	    	return list_node_allocator::allocate();
	    }
        
        // 释放一个节点
        void put_node(link_type x) {
        	list_node_allocator::deallocate(x);
        }
        
        // 产生一个节点，带有元素值
        link_type create_node(const T& x) {
        	link_type newListNode = get_node();
        	construct(&newListNode->data, x); // 全局函数，构造析构基本工具
        	return newListNode;
        }
        
        // 销毁(析构并释放)一个节点
        void destroy_node(link_type d) {
            destroy(&d->data); // 全局函数，构造析构基本工具
            put_node(d);
        }

    protected:
	    link_type node; // 只要一个指针，便可表示整个环状双向链表
                        // 刻意在环状链表的尾端加上一个空白节点
                        // 以符合STL规范之 “前闭后开”区间
	    void empty_initialize() {
	    	node = get_node();  // 配置一个节点空间，令node指向它
	    	node->next = node;  // 令node头尾都指向自己，不设元素值
	    	node->prev = node;
	    }

	public:
	    //构造函数
	    list() { empty_initialize(); } //产生空的链表
        list( initializer_list<T> l) {
            empty_initialize();
            auto iter = l.begin();
            for (; iter != l.end(); ++iter)
                push_back(*iter);
        }

        // To Do：没有定义拷贝构造函数，拷贝时会出错！
        list(const list<T, Alloc>& x)
        { 
            empty_initialize();
            iterator iter = x.begin();
            for (; iter != x.end(); ++iter)
                push_back(*iter); 
        }

        list(const T* first, const T* last)
        { 
            empty_initialize();
            const T* p = first;
            for (; p != last; ++p)
                push_back(*p); 
        }

        ~list() {
            clear();
            erase(end());
        }

        iterator begin() const { return node->next; }
	    iterator end() const { return node; }
	    bool empty() const { return node->next == node; }
	    size_type size() const {
	    	size_type result = 0;
	    	result = distance(begin(), end());
	    	return result;
	    }
	    reference front() { return *begin(); }
	    reference back() { return *(--end()); }
        
        void push_back(const T& x) { insert(end(), x); }
        void push_front(const T& x) { insert(begin(), x); }
        void pop_front() { erase(begin()); }
        void pop_back() { erase(--end()); }
        
        iterator insert(iterator position, const T& x) {
        	link_type tmp = create_node(x);
        	tmp->next = position.node;
        	tmp->prev = position.node->prev;
        	position.node->prev->next = tmp;
        	position.node->prev = tmp;
        	return tmp;
        }

        iterator erase(iterator position) {
        	link_type next_node = position.node->next;
        	link_type prev_node = position.node->prev;
            prev_node->next = next_node;
            next_node->prev = prev_node;
            destroy_node(position.node);
            return iterator(next_node);
        }

        // 清除所有list节点
        void clear() {
            link_type cur = node->next;
            while (cur != node) {  // 遍历每一个节点
            	link_type tmp = cur;
                cur = cur->next;
                destroy_node(tmp);  // 销毁（析构并释放）一个节点
            }
            // 恢复 node 原始状态
            node->next = node;
            node->prev = node;
        }
        
        // 删除所有值为value的节点
        void remove(const T& value) {
            iterator cur = begin();
            while (cur != end()) {
            	iterator tmp = cur;
                ++cur;
                if (*tmp == value) erase(tmp);
            }
        }
        
        // 移除数值相同的连续元素。注意，只有
        void unique() {
            iterator first = begin();
            iterator last  = end();
            if (first == last) return;
            iterator next = first;
            while (++next != last) { // 双指针
            	if (*first == *next) erase(next);
            	else first = next;
            	next = first;
            }
        }

        // 将 [first,last) 内的所有元素移动到 position 之前
        void transfer(iterator position, iterator first, iterator last) {
        	if (last == position) return;
            last.node->prev->next = position.node;
            first.node->prev->next = last.node;
            position.node->prev->next = first.node;
            link_type tmp = position.node->prev;
            position.node->prev = last.node->prev;
            last.node->prev = first.node->prev;
            first.node->prev = tmp; 
        }
        
        // 将x接合于position所指位置之前。x必须不同于*this
        void splice(iterator position, list& x) {
        	if(!x.empty())
                transfer(position, x.begin(), x.end());
        }

        // 将i所指元素接合于position所指位置之前。position和i可指向同一个list
        void splice(iterator position, iterator i) {
            iterator j = i;
            ++j;
            if (position == i || position == j) return;
            transfer(position, i, j);
        }

        // 将 [first,last) 内的所有元素接合于position所指位置之前
        // position 和 [first,last) 可指向同一个 list
        // 但 position 不能位于 [first,last) 内
        void splice(iterator position, iterator first, iterator last) {
            if (first != last)
        	    transfer(position, first, last);
        }

        // merge将x合并到*this。两个lists的内容都必须先经过递增排序
        void merge(list& x) {
           iterator first1 = begin();
           iterator last1 = end();
           iterator first2 = x.begin();
           iterator last2 = x.end();

            while (first1 != last1 && first2 != last2)
            {
                if (*first2 < *first1)
                {
                    iterator next = first2;
                    transfer(first1, first2, ++next);
                    first2 = next;
                }
                else ++first1;
            }

            if (first2 != last2) transfer(last1, first2, last2);
        }

        // reverse 将 *this 的内容逆向重置
        void reverse() {
            // 如果是空链表或者仅有一个元素，就不进行任何操作
            // 也可以使用size来判断，但比较慢
            if (node->next == node || node->next->next == node) return;

            iterator first = begin();
            ++first;
            while(first != end()) {
                iterator old = first;
                ++first;
                transfer(begin(), old, first);
            }
        }
        
        void swap(list& x) {
            link_type tmp = x.node;
            x.node = this->node;
            this->node = tmp;
        }

        /*
        // list 不能使用 STL 算法 sort()，必须使用自己的 sort() member function
        // 因为 STL 算法 sort 只接受 RamdonAccessIterator
        // quick sort
        void sort()
        {
            // 如果是空链表或者仅有一个元素，就不进行任何操作
            // 也可以使用size来判断，但比较慢
            if (node->next == node || node->next->next == node) return;

            // 一些新的lists，作为中介数据存放区
            list<T> carry;
            list<T> counter[64];
            int fill = 0;
            while (!empty()) {
                carry.splice(carry.begin(), *this, begin());
                int i = 0;
                while (i < fill && !counter[i].empty()) {
                    counter[i].merge(carry);
                    carry.swap(counter[i++]);
                }
                carry.swap(counter[i]);
                if (i == fill) ++fill;
            }

            for (int i = 1; i < fill; ++i)
                counter[i].merge(counter[i-1]);
            swap(counter[fill-1]);
        }
        */

        // 链表插入排序    
       void sort() {
            // 如果是空链表或者仅有一个元素，就不进行任何操作
            // 也可以使用size来判断，但比较慢
            if (node->next == node || node->next->next == node) return;
            
            list<T, Alloc> tmp;
            iterator first = begin();
            while(!empty()) {
                iterator cur = tmp.begin();
                while(cur != tmp.end() && *cur < *first) ++cur;
                tmp.splice(cur, first);
                first = begin();
            }

            swap(tmp);
        }
	};
}

#endif