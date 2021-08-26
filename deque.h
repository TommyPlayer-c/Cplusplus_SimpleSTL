#ifndef _SIMPLE_STL_DEQUE_H_
#define _SIMPLE_STL_DEQUE_H_

#include <cstddef>
#include "./memory.h"
#include "./stl_iterator.h"


namespace SimpleSTL {
	template <class T, class Ref, class Ptr, size_t BufSize>
	struct __deque_iterator {
		typedef __deque_iterator<T, T&, T*, BufSize> iterator;
		typedef __deque_iterator<const T,const T&,const T*, BufSize> const_iterator;
		// 单个缓冲区的大小（T的个数）
        static size_t buffer_size() { return __deque_buf_size(BufSize, sizeof(T)); }

		// 未继承 std::iterator，所以必须自行撰写五个必要的迭代器相应型别（第3章）
		typedef random_access_iterator_tag iterator_category;   // (1)
		typedef T value_type;                                   // (2)
		typedef Ptr pointer;                                    // (3)
		typedef Ref reference;                                  // (4)
		typedef size_t size_type;                               
		typedef ptrdiff_t difference_type;                      // (5)
		typedef T** map_pointer;

		typedef __deque_iterator self;

		// 保持与容器的联结
		T* cur;         // 此迭代器所指缓冲区中的现行（current）元素
		T* first;       // 此迭代器所指缓冲区的头
		T* last;        // 此迭代器所指缓冲区的尾
		map_pointer node;   // 指向管控中心
        
        // 如果 n 不为 0，传回 n，表示 buffer size 由用户自定义
        // 如果 n 为 0，表示 buffer size 使用默认值，那么
        //      如果 sz（元素大小，sizeof(value_type)）小于 512，传回 512/sz
        //      如果 sz 不小于 512，传回 1
        static size_t __deque_buf_size(size_t n, size_t sz) {
        	return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
        }

        // 将此迭代器指向新缓冲区
		void set_node(map_pointer new_node) {
			node = new_node;
			first = *new_node;
			last = first + difference_type(buffer_size());
		}

		reference operator*() const { return *cur; }
		pointer operator->() const { return &(operator*()); }
		difference_type operator-(const self& x) const {
			return difference_type(buffer_size()) * (node - x.node - 1) +
			    (cur - first) + (x.last - x.cur);
		}

        // prefix form
		self& operator++() {
			++cur;
			if(cur == last) {       // 如果已到达所在缓冲区的尾端
				set_node(node + 1); // 就切换至下一个缓冲区
				cur = first;        // 的第一个元素
			}
			return *this;
		}

        // postfix form
		self& operator++(int) {  
            self tmp = *this;
            ++*this;
            return tmp;
		}

        // prefix form
		self& operator--() {
			if (cur == first) {     // 如果已到达缓冲区的头端
				set_node(node - 1); // 就切换至前一缓冲区
				cur = last;         // 的最后一个元素
			}
			--cur;
			return *this;
		}
        
        // postfix form
        self& operator--(int) {
			self tmp = *this;
			--*this;
			return tmp;
		}
        
        // 以下实现随机存取。迭代器可以直接跳跃 n 个距离
        self& operator+=(difference_type n) {
            difference_type offset = n + cur - first;
        	// 目标位置在同一缓冲区内
            if(offset >= 0 && offset < (difference_type)buffer_size()) {
        		cur += n;
        		return *this;
        	}
            else {
            // 目标位置不在同一缓冲区内
                difference_type node_offset = 
                    offset > 0 ? offset / difference_type(buffer_size())
                    : -difference_type((-offset - 1) / buffer_size()) - 1;  // 这个分类计算不好想
                // 切换至正确的缓冲区
                set_node(node + node_offset);
                // 切换至正确的元素
                cur = first + (offset - node_offset * difference_type(buffer_size()));
            }
            return *this;
        }
        
        self operator+(difference_type n) const {
        	self tmp = *this;
        	return tmp += n;    // 调用+=
        }

        self& operator-=(difference_type n) { return *this += -n;}
        // 以上利用 += 完成 -=

        self operator-(difference_type n) const {
        	self tmp = *this;
        	return tmp -= n;    // 调用-=
        }
        
        // 以下实现随机存取。迭代器可以直接跳跃 n 个距离。
        // 调用了 operator*，operator+
        reference operator[](difference_type n) const { return *(*this + n); }
        bool operator==(const self& x) const { return cur == x.cur; }
        bool operator!=(const self& x) const { return !(*this == x); }
        bool operator<(const self& x) const {
        	return (node == x.node) ? (cur < x.cur) : (node < x.node);
        }
	};


    template<class T, class Alloc = alloc2, size_t BufSize = 0>
	class deque{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef size_t size_type;
        typedef ptrdiff_t difference_type;

    public:
    	typedef __deque_iterator<T, T&, T*, BufSize> iterator;
          
	protected:
		// 元素的指针的指针（pointer of pointer of T）
		typedef pointer *map_pointer;

    protected:
        iterator start;         // 第一个节点
        iterator finish;        // 最后一个节点
		map_pointer map;        // 指向map，map是块连续空间，其每个元素都是指针，指向一个缓冲区
		size_type map_size;     // map内有多少指针
		static const int initial_map_size = 8;
		
	public:
		iterator begin() { return start; }
        iterator end() { return finish; }
        reference operator[] (size_type n) {
        	return start[difference_type(n)];   // 调用__deque_iterator<>::operator[]
        }

        reference front() {return *start;}      // 调用__deque_iterator<>::operator*
        reference back() {
        	return *(finish - 1);
        }

        size_type size() const { return finish - start; }
        // 以下调用 interator::operator-
        size_type max_size() const { return size_type(-1);}
        bool empty() const { return finish == start;}

    /******************** 4.4.5 deque的构造与内存管理 ********************/
	protected:
		// 专属空间配置器，每次配置一个元素大小
        typedef simple_alloc<value_type, Alloc> data_allocator;
        // 专属空间配置器，每次配置一个指针大小
        typedef simple_alloc<pointer, Alloc> map_allocator;
        
        void fill_initialize(size_type n, const value_type& value);
        void create_map_and_nodes(size_type num_elements);
        pointer allocate_node() {
            return data_allocator::allocate(buffer_size());
        }
		void deallocate_node(T* p) {
            data_allocator::deallocate(p);
        }

        static size_t buffer_size() {return __deque_buf_size(BufSize, sizeof(T));}
        static size_t __deque_buf_size(size_t n, size_t sz) {
            return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
        }
        

    public:
        deque(int n, const value_type& value)
           : start(), finish(), map(0), map_size(0) {
               fill_initialize(n, value);
           }
        
        deque() : start(), finish(), map(0), map_size(0){
            create_map_and_nodes(0);
        }
        
        ~deque() {}

        void push_back(const value_type& t) {
        	// 最后缓冲区尚有两个（含）以上的元素备用空间
            if (finish.cur != finish.last - 1) {
        		construct(finish.cur, t);   // 直接在备用空间回闪构造元素
        		++finish.cur;
        	} 
            else {  // 最后缓冲区只剩一个元素备用空间
        		push_back_aux(t);
        	}
        } 

        void push_front(const value_type& t){
            if (start.cur != start.first) { // 第一缓冲区尚有备用空间
                construct(start.cur - 1, t);   // 直接在备用空间回闪构造元素    
                --start.cur;
            } 
            else { //第一缓冲区已无备用空间了
                push_front_aux(t);
            }
        }

        void pop_back() {
            if (finish.cur != finish.first) {
                // 最后缓冲区有一个（或更多）元素
				--finish.cur;
                destroy(finish.cur);
            } 
			else {
                // 最后缓冲区没有任何元素
				pop_back_aux();
            }
        }

		void pop_front() {
            if (start.cur != start.last - 1) {
                // 第一个缓冲区有两个（或更多）元素
                destroy(start.cur);
				++start.cur;
            } 
			else {
                // 第一缓冲区仅有一个元素
				pop_front_aux();
            }
        }

        void clear();

		// 清除某个元素，P164
		iterator erase(iterator pos) {
    		iterator next = pos;
    		++next;
    		difference_type index = pos - start;
    		if (size_type(index) < (this->size() >> 1)) {
    		  	copy_backward(start, pos, next);
    		  	pop_front();
    		}
    		else {
    		  	copy(next, finish, pos);
    		  	pop_back();
    		}
    		return start + index;
  		}
		iterator erase(iterator first, iterator last);

		iterator insert(iterator position, const value_type& x) {
  			if (position.cur == start.cur) {
  		    	push_front(x);
  		    	return start;
  		  	}
  		  	else if (position.cur == finish.cur) {
  		  	  	push_back(x);
  		  	  	iterator tmp = finish;
  		  	  	--tmp;
  		  	  	return tmp;
  		  	}
  		  	else {
  		  	  	return insert_aux(position, x);
  		  	}
  		}
		iterator insert_aux(iterator pos, const value_type& x);

    protected:
        void push_back_aux(const value_type& t);
        void push_front_aux(const value_type& t);
        void pop_back_aux();
        void pop_front_aux();

        void reserve_map_at_back(size_type node_to_add = 1);
        void reserve_map_at_front(size_type node_to_add = 1);
        void reallocate_map(size_type node_to_add, bool add_at_front);
	};


    template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::fill_initialize(size_type n,
		                                const value_type& value) {
		create_map_and_nodes(n);    // 把deque的结构都产生并安排好
		map_pointer cur;
        // 为每个节点的缓冲区设定初值
		for (cur = start.node; cur < finish.node; ++cur)
			uninitialized_fill(*cur, *cur + buffer_size(), value);
		// 最后一个节点的设定稍有不同（因为尾端可能有备用空间，不必设初值）
        uninitialized_fill(finish.first, finish.cur, value);
	}

    // 负责产生并安排好 deque 的结构
    template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::create_map_and_nodes(size_type num_elements) {
	    // 需要节点数=(元素个数/每个缓冲区可容纳的元素个数+1)
        // 如果刚好整除，会多分配一个节点
		size_type num_nodes = num_elements / buffer_size() + 1;
		
        // 一个map要管理几个节点。最少8个，最多是 “所需节点数加2”
        // （前后各预留一个，扩充时可用）
		map_size = initial_map_size > num_nodes + 2 ? initial_map_size : num_nodes + 2;
		map = map_allocator::allocate(map_size);
		
        // 以上配置出一个 “具有map_size节点” 的 map

        // 以下令nstart和nfinish指向map所拥有之全部节点的最中央区段
		map_pointer nstart = map + (map_size - num_nodes) / 2;
		map_pointer nfinish = nstart + num_nodes - 1;
		
        map_pointer cur;
        // 为map内每个现用节点配置缓冲区。所有缓冲区加起来就是deque的
        // 可用空间（最后一个缓冲区可能留有一些余裕）
		for (cur = nstart; cur <= nfinish; cur++)
			*cur = allocate_node();

	    start.set_node(nstart);
	    finish.set_node(nfinish);
	    start.cur = start.first;
	    finish.cur = finish.first + num_elements % buffer_size();
        // 如果刚好整除，会多配一个节点
        // 此时即令cur指向这多配的一个节点的起始处
	}

    // 只有当最后一个缓冲区只剩一个备用元素空间时才会调用
    template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::push_back_aux(const value_type& t) {
		value_type t_copy = t;
		reserve_map_at_back();      // 判断是否需要重换一个map
		*(finish.node + 1) = allocate_node();  // 配置一个新缓冲区

		construct(finish.cur, t_copy);
		finish.set_node(finish.node + 1);   // 改变finish，令其指向新节点
		finish.cur = finish.first;
	}

    // 只有当第一缓冲区没有任何备用元素时才会被调用
    template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::push_front_aux(const value_type& t) 
    {
	    value_type t_copy = t;
	    reserve_map_at_front();      // 判断是否需要重换一个map
	    *(start.node - 1) = allocate_node();  // 配置一个新缓冲区    

	    start.set_node(start.node - 1);   // 改变start，令其指向新节点
	    start.cur = start.last - 1;
	    construct(start.cur, t_copy);
	}


    template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::reserve_map_at_back(size_type node_to_add) {
	    if (node_to_add + 1 > map_size - (finish.node - map)) {
	    	reallocate_map(node_to_add, false);
	    }
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::reserve_map_at_front(size_type node_to_add) {
	    if (node_to_add > start.node - map) {
	    	reallocate_map(node_to_add, true);
	    }
	}

	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::reallocate_map(size_type node_to_add, 
		                                        bool add_at_front) {
	    size_type old_num_nodes = finish.node - start.node + 1;
	    size_type new_num_nodes = old_num_nodes + node_to_add;

	    map_pointer new_nstart;
	    if (map_size > 2 * new_num_nodes) {
	    	//当前内存在map的右半边，需要将其移到正中间来（不用配置新空间）
	    	new_nstart = map + (map_size - new_num_nodes) / 2 
	    	             + (add_at_front ? node_to_add : 0);
	    	if (new_nstart < start.node)
	    		copy(start.node, finish.node + 1, new_nstart);
	    	else
	    		copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
	    } 
        else {
	    	size_type new_map_size = map_size + max(map_size, node_to_add) + 2;
	    	// 配置一块空间，准备给新 map 使用
			map_pointer new_map = map_allocator::allocate(new_map_size);
	    	new_nstart = new_map + (new_map_size - new_num_nodes) / 2
	    	            + (add_at_front ? node_to_add : 0);
	    	// 把原 map 内容拷贝过来，注意 copy 是左闭右开的
			copy(start.node, finish.node + 1, new_nstart);
			// 释放原 map
	    	map_allocator::deallocate(map, map_size);

	    	map = new_map;
	    	map_size = new_map_size;
	    }

	    start.set_node(new_nstart);
	    finish.set_node(new_nstart + old_num_nodes - 1);
	}
	
	// 只有当 finish.cur == finish.first 时才会被调用
	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::pop_back_aux(){
		deallocate_node(finish.first);		// 释放最后一个缓冲区
		finish.set_node(finish.node - 1);
		finish.cur = finish.last - 1;
		destroy(finish.cur);
	}

	// 只有当 start.cur == start.last-1 时才会被调用
	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::pop_front_aux(){
		destroy(start.cur);					 // 将第一个缓冲区的唯第一个（也是最后一个）元素析构		
		deallocate_node(start.last);		 // 释放第一个缓冲区
		start.set_node(start.node + 1);
		start.cur = start.first;
		
	}

	// 注意，最终需要保留一个缓冲区。这是 deque 的策略，也是 deque 的初始状态
	template<class T, class Alloc, size_t BufSize>
	void deque<T, Alloc, BufSize>::clear() {
		// 以下针对头尾以外的每一个缓冲区（它们一定都是饱满的）
		for (map_pointer node = start.node + 1; node < finish.node; ++node) {
			// 将缓冲区内所有元素析构
			destroy(*node, *node + buffer_size());
			// 释放缓冲区内存
			data_allocator::deallocate(*node, buffer_size());
		}

		if (start.node != finish.node) {	// 至少有头尾两个缓冲区
			destroy(start.cur, start.last);
			destroy(finish.first, finish.cur);
			// 以下释放尾缓冲区，注意，头缓冲区保留
			data_allocator::deallocate(finish.first, buffer_size());
		} 
		else { // 只有一个缓冲区
	        destroy(start.cur, finish.cur);		// 将此唯一缓冲区内的所有元素析构
			// 注意，并不释放缓冲区空间，这唯一的缓冲区将保留
		}
		
		finish = start;
	}

	template <class T, class Alloc, size_t BufSize>
	typename deque<T, Alloc, BufSize>::iterator 
	deque<T, Alloc, BufSize>::erase(iterator first, iterator last)
	{
	  	if (first == start && last == finish) {
	  	  	clear();
	  	  	return finish;
	  	}
	  	else {
	  	  	difference_type n = last - first;
	  	  	difference_type elems_before = first - start;
	  	  	// 如果区间前方的元素比较少，清除区间前方的元素。
			if (elems_before < difference_type((this->size() - n) / 2)) {
	  	    	copy_backward(start, first, last);
	  	    	iterator new_start = start + n;
	  	    	destroy(start, new_start);
	  	    	for (map_pointer cur = start.node; cur < new_start.node; ++cur)
					data_allocator::deallocate(*cur, buffer_size());
	  	    	start = new_start;
	  	  	}
	  	  	else {
	  	  	  	copy(last, finish, first);
	  	  	  	iterator new_finish = finish - n;
	  	  	  	destroy(new_finish, finish);
	  	  	  	for (map_pointer cur = new_finish.node; cur <= finish.node; ++cur)
					data_allocator::deallocate(*cur, buffer_size());
	  	  	  	finish = new_finish;
	  	  	}
	  	  	return start + elems_before;
		}
	}

	template <class T, class Alloc, size_t BufSize>
	typename deque<T, Alloc, BufSize>::iterator
	deque<T, Alloc, BufSize>::insert_aux(iterator pos, const value_type& x)
	{
	  	difference_type index = pos - start;
	  	value_type x_copy = x;
	  	if (size_type(index) < this->size() / 2) {
	  	  	push_front(front());
	  	  	iterator front1 = start;
	  	  	++front1;
	  	  	iterator front2 = front1;
	  	  	++front2;
	  	  	pos = start + index;
	  	  	iterator pos1 = pos;
	  	  	++pos1;
	  	  	copy(front2, pos1, front1); // 前面的元素向前挪一个位置
	  	}
	  	else {
	  	  	push_back(back());
	  	  	iterator back1 = finish;
	  	  	--back1;
	  	  	iterator back2 = back1;
	  	  	--back2;
	  	  	pos = start + index;
	  	  	copy_backward(pos, back2, back1);	// 后面的元素向后挪一个位置
	  	}

	  	*pos = x_copy;
	  	return pos;
	}
}
#endif