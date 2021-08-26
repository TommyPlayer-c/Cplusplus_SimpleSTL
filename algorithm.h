#ifndef _SIMPLE_STL_ALGORITHM_H_
#define _SIMPLE_STL_ALGORITHM_H_
// 参考 http://www.cplusplus.com

namespace SimpleSTL
{
    template <class InputIterator, class OutputIterator>
    OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
    {
        while (first != last)
        {
            *result = *first;
            ++result;
            ++first;
        }
        return result;
    }

    template <class BidirectionalIterator1, class BidirectionalIterator2>
    BidirectionalIterator2 copy_backward(BidirectionalIterator1 first,
                                         BidirectionalIterator1 last,
                                         BidirectionalIterator2 result)
    {
        while (last != first)
            *(--result) = *(--last);
        return result;
    }

    template <class ForwardIterator, class T>
    void fill(ForwardIterator first, ForwardIterator last, const T &val)
    {
        while (first != last)
        {
            *first = val;
            ++first;
        }
    }

    template <class OutputIterator, class Size, class T>
    OutputIterator fill_n(OutputIterator first, Size n, const T &val)
    {
        while (n > 0)
        {
            *first = val;
            ++first;
            --n;
        }
        return first; // since C++11
    }
}

#endif
