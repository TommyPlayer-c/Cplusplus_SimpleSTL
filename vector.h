#ifndef _SIMPLE_STL_VECTOR_H_
#define _SIMPLE_STL_VECTOR_H_

#include <initializer_list>
#include "./type_traits.h"
#include "./memory.h"
#include "./algorithm.h"
#include "./stl_iterator.h"

namespace SimpleSTL {
	template<class T, class Alloc = alloc2>
	class vector {
	public:
		// vector 的嵌套型别定义
		typedef T           value_type;
		typedef value_type* pointer;
		typedef value_type* iterator;
		typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef size_t      size_type;
        typedef ptrdiff_t   difference_type;  
    	
    protected:
    	// simple_alloc是SGI STL的空间配置器
    	typedef simple_alloc<value_type, Alloc> data_allocator;

		iterator start;            //表示目前使用空间头
		iterator finish;           //表示目前使用空间尾
		iterator end_of_storage;   //表示目前可用空间的尾

		iterator insert_aux(iterator position, const T& x);
		void deallocate() {
			if (start) {
				data_allocator::deallocate(start, end_of_storage - start);
			}
		}
        
        void fill_initialize(size_type n, const T& value) {
        	start = allocate_and_fill(n, value);
        	finish = start + n;
        	end_of_storage = finish;
        }

    public:
        iterator begin() const { return start; }
        iterator end() const  { return finish; }
        size_type size() const { return size_type(end() - begin()); }
        size_type capacity() const { return size_type(end_of_storage - begin()); }
        bool empty() const { return begin() == end(); }
        reference operator[](size_type n) { return *(begin() + n); }

        vector() : start(0), finish(0), end_of_storage(0) {}
        vector(size_type n, const T& value) { fill_initialize(n, value);}
        vector(int n, const T& value) { fill_initialize(n, value);} 
        vector(long n, const T& value) { fill_initialize(n, value);}
        vector(const std::initializer_list<T> v) {
            auto _start = v.begin();
            auto _end = v.end();
            size_type n = v.size();
            fill_initialize(n, T());
            finish = SimpleSTL::copy(_start, _end, start);            
        }
        vector(T* first, T* last) {
            auto _start = first;
            auto _end = last;
            size_type n = last - first + 1;
            fill_initialize(n, T());
            this->finish = SimpleSTL::copy(_start, _end, start);
        }
        explicit vector(size_type n) { fill_initialize(n, T()); }

        // TO DO 缺少一个拷贝构造函数，需要补写
        vector(const vector<T, Alloc>& __x) 
        { 
            size_type n = __x.size();
            fill_initialize(n, T());
            finish = uninitialized_copy(__x.begin(), __x.end(), start); 
        }

        ~vector() {
        	destroy(start, finish);     // stl_construct.h中的全局函数
        	deallocate();               // member function
        }

        reference front() { return *begin(); }          // 第一个元素
        const_reference front() const { return *begin(); }  // 重载为const成员
        reference back() { return *(end() - 1); }       // 最后一个元素
        void push_back(const T& x) {                    // 将元素插入至最尾端
            if (finish != end_of_storage) {
            	construct(finish, x);                   // 全局函数
            	++finish;
            } 
            else {
            	insert_aux(end(), x);                   // member function
            }
        }

        void pop_back() {
        	--finish;
        	destroy(finish);
        }
     
        iterator erase(iterator position);
        iterator erase(iterator first, iterator last);
        
        void resize(size_type new_size, const T& x) {
        	if (new_size < size()) 
        		erase(begin() + new_size, end());
        	else
        		insert(end(), new_size - size(), x);
        }
        
        void resize(size_type new_size) { resize(new_size, T());}
        void clear() { erase(begin(), end());}
        void reserve(size_type n) {
            if (capacity() < n) {
                const size_type old_size = size();
                iterator tmp = allocate_and_copy(n, start, finish);
                destroy(start, finish);
                data_allocator::deallocate(start, end_of_storage - start);
                start = tmp;
                finish = tmp + old_size;
                end_of_storage = start + n;
            }
        }
        
        void swap(vector<T>& __x) {
            std::swap(start, __x.start);
            std::swap(finish, __x.finish);
            std::swap(end_of_storage, __x.end_of_storage);
        }

        iterator insert(iterator position, const T& x);
        iterator insert(iterator position, size_type n, const T& x);
        iterator insert(iterator position, iterator first, iterator last);

    protected:
        iterator allocate_and_fill(size_type n, const T& x) {
        	iterator result = data_allocator::allocate(n);
            //在获取到的内存上创建对象
            uninitialized_fill_n(result, n, x);
            return result;
        }

        template <class ForwardIterator>
        iterator allocate_and_copy(size_type n, ForwardIterator first, 
                                                       ForwardIterator last)
        {
            iterator result = data_allocator::allocate(n);
            uninitialized_copy(first, last, result);
            return result;
        }
	};


    /**************************** erase ****************************/
    // 给人感觉erase就是移动元素的位置
    // 清除某个位置上的元素
    template<class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator position) {
    	if (position + 1 != end())
    		SimpleSTL::copy(position + 1, finish, position);
        --finish;
    	destroy(finish);
    	return position;
    }

    // 清除[first, last)中的所有元素
    template<class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::erase
    (iterator first, iterator last) {
	    iterator i = SimpleSTL::copy(last, finish, first);
        destroy(i, finish);
        finish -= (last - first);
        return first;
    }

    /**************************** insert ****************************/
    // single element (1)	
    template<class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(
        iterator position, const T& x)
    {
        // return insert_aux(position, x); 有bug
        return insert(position, 1, x);
    }
    

    // fill(2)
    template<class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(
        iterator position, size_type n, const T& x) 
    {
        if (n <= 0) return position;
        if (n != 0 && size_type(end_of_storage - finish) >= n) {
            // if (size_type(end_of_storage - finish) >= n) { //备用空间大于等于新增元素个数                
                /*
                T x_copy = x;
                const size_type elems_after = finish - position;
                iterator old_finish = finish;
                if (elems_after > n) {  // 插入点之后的现有元素大于新增元素个数
                    SimpleSTL::uninitialized_copy(finish - n, finish, finish);
                    finish += n;
                    SimpleSTL::copy_backward(position, old_finish - n, old_finish);
                    SimpleSTL::fill(position, position + n, x_copy);  // 从插入点开始填入新值
                }
                else { // 插入点之后的现有元素小于新增元素个数
                    SimpleSTL::uninitialized_fill_n(finish, n - elems_after, x_copy);
                    finish += n - elems_after;
                    SimpleSTL::uninitialized_copy(position, old_finish, finish);
                    finish += elems_after;
                    SimpleSTL::fill(position, old_finish, x_copy);  // 从插入点开始填入新值
                }*/
            
            // 书中的算法过于繁琐，下面的逻辑很简单
            T x_copy = x;
            uninitialized_fill_n(finish, n, x_copy);    // 初始化未初始化内存
            // 注意这里要使用copy_backword，不然会覆盖后面要移动的值
            SimpleSTL::copy_backward(position, finish, finish + n);             
            SimpleSTL::fill(position, position + n, x_copy);               
            finish += n;
            return position;
        } 
        else {
            // 备用空间小于新增元素个数（必须配置额外的内存）
            const size_type old_size = size();
            // const size_type new_size = old_size + std::max(old_size, n);
            const size_type new_size = old_size != 0 ? 2 * old_size : 1;
            // 以下配置新的 vector 空间
            iterator new_start = data_allocator::allocate(new_size);
            iterator new_finish = new_start;
            try {
                // 首先将旧vector的插入点之前的元素复制到新空间
                if (start != position)
                    new_finish = uninitialized_copy(start, position, new_start);                   
                // 再将新增元素（初值皆为n）填入新空间
                new_finish = uninitialized_fill_n(new_finish, n, x);
                // 再将旧vector的插入点之后的元素复制到新空间
                new_finish = uninitialized_copy(position, finish, new_finish);                               
            }
            catch(...) {
                // 如有异常发生，实现“commit or rollback” semantics
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, new_size);
                throw;
            }
            
            // 以下清除并释放旧的vector
            destroy(start, finish);
            deallocate();
            iterator ret = new_start + (position - start);
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + new_size;
            
            return ret;
        }
    }

    /*  这个算法使用 insert_aux，简单但不高效。
    // fill(2)
    template<class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(
        iterator position, size_type n, const T& x)
    {
        iterator iter = position;
        while (n--)
        {
            iter = insert_aux(iter, x);
        }
        return iter;
    }
    */

    // range (3)	
    template<class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(
        iterator position, iterator _first, iterator _last)
    {
        int n = _last - _first;
        if (n <= 0) return position;
        if (size_type(end_of_storage - finish) >= n)
        {
            uninitialized_fill_n(finish, n, T());    // 初始化未初始化内存
            // 注意这里要使用copy_backword，不然会覆盖后面要移动的值
            SimpleSTL::copy_backward(position, finish, finish + n);               
            iterator cur = position;
            for (int i = 0; i < n; ++i)
                *(cur+i) = *(_first++);                
            finish += n;
            return position;
           /*
            const size_type elems_after = finish - position;
            iterator old_finish = finish;
            if (elems_after > n) {  // 插入点之后的现有元素大于新增元素个数
                SimpleSTL::uninitialized_copy(finish - n, finish, finish);
                finish += n;
                SimpleSTL::copy_backward(position, old_finish - n, old_finish);
                // SimpleSTL::fill(position, position + n, x_copy);  // 从插入点开始填入新值
                iterator cur = position;
                for (int i = 0; i < n; ++i)
                    *(cur+i) = *(_first++);
            }
            else { // 插入点之后的现有元素小于新增元素个数
                SimpleSTL::uninitialized_fill_n(finish, n - elems_after, T());
                finish += n - elems_after;
                SimpleSTL::uninitialized_copy(position, old_finish, finish);
                finish += elems_after;
                // SimpleSTL::fill(position, old_finish, x_copy);  // 从插入点开始填入新值
                iterator cur = position;
                for (int i = 0; i < n; ++i)
                    *(cur+i) = *(_first++);
            }
            */
        }
        else {
            // 备用空间小于新增元素个数（必须配置额外的内存）
            const size_type old_size = size();
            // const size_type new_size = old_size + std::max(old_size, n);
            const size_type new_size = old_size != 0 ? 2 * old_size : 1;
            // 以下配置新的 vector 空间
            iterator new_start = data_allocator::allocate(new_size);
            iterator new_finish = new_start;
            try {
                // 1.首先将旧vector的插入点之前的元素复制到新空间
                new_finish = uninitialized_copy(start, position, new_start);  
                // 2.再将新增元素填入新空间
                new_finish = uninitialized_fill_n(new_finish, n, T());
                iterator cur = new_finish - n;  // 这里必须要减n，前面new_finish的值由于填充增加了。
                for (int i = 0; i < n; ++i) { 
                    *(cur++) = *(_first++);
                }
                // 3.再将旧vector的插入点之后的元素复制到新空间
                new_finish = uninitialized_copy(position, finish, new_finish);                                   
            }
            catch(...) {
                // 如有异常发生，实现“commit or rollback” semantics
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, new_size);
                throw;
            }
            
            // 以下清除并释放旧的vector
            destroy(start, finish);
            deallocate();
            iterator ret = new_start + (position - start + 1);
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + new_size;
            
            return ret;
        }
    }

    
    /**************************** insert_aux ****************************/
    template<class T, class Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::insert_aux(
        iterator position, const T& x) {
        if (finish != end_of_storage) { // 还有备用空间            
            construct(finish, *(finish - 1));   //在备用空间起始处构建一个对象，并以vector的最后一个元素值为其初值
            ++finish;
            T x_copy = x;
            SimpleSTL::copy_backward(position, finish - 2, finish - 1);
            *position = x_copy;
            
            return position;
        } 
        else {  // 已无备用空间
            const size_type old_size = size();
            const size_type new_size = old_size != 0 ? 2 * old_size : 1;
            // 以上配置原则：如果远大小为0，则配置1；如果原大小不为0，则配置为原大小的两倍
            // 前半段用来放置原数据，后半段准备用来放置新数据
            
            iterator new_start = data_allocator::allocate(new_size);
            iterator new_finish = new_start;
            try {
                new_finish = uninitialized_copy(start, position, new_start);
                construct(new_finish, x);
                ++new_finish;
                // 将安插点之后的原内容拷贝过来（提示：本函数也可能被insert(p,x)调用）
                new_finish = uninitialized_copy(position, finish, new_finish);
            } 
            catch (...) {
                // "commit or rollback" semantics
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, new_size);
                throw;
            }

            destroy(begin(), end());
            deallocate();
            iterator ret = new_start + (position - start);
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + new_size; 

            return ret;
        }
    }
}

#endif