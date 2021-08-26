#include <iostream>
#include "./deque.h"

using namespace std;
using namespace SimpleSTL;

int main()
{
    deque<int> ideq(20, 9);
    cout << "size=" << ideq.size() << endl;

    for (int i = 0; i < ideq.size(); ++i)
        ideq[i] = i;

    for (int i = 0; i < ideq.size(); ++i)
        cout << ideq[i] << ' ';
    cout << endl;

    for (int i = 0; i < 3; ++i)
        ideq.push_back(i);
    
    for (int i = 0; i < ideq.size(); ++i)
        cout << ideq[i] << ' ';
    cout << endl;
    cout << "size=" << ideq.size() << endl;

    ideq.push_back(3);
    for (int i = 0; i < ideq.size(); ++i)
        cout << ideq[i] << ' ';
    cout << endl;
    cout << "size=" << ideq.size() << endl;

    ideq.push_front(99);
    for (int i = 0; i < ideq.size(); ++i)
        cout << ideq[i] << ' ';
    cout << endl;
    cout << "size=" << ideq.size() << endl;

    ideq.push_front(98);
    ideq.push_front(97);
    for (int i = 0; i < ideq.size(); ++i)
        cout << ideq[i] << ' ';
    cout << endl;
    cout << "size=" << ideq.size() << endl;
}