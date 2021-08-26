#include <iostream>
#include "./queue.h"
#include "./list.h"

using namespace std;
using namespace SimpleSTL;

int main() {
    queue<int> iqueue;
    iqueue.push(1);
    iqueue.push(3);
    iqueue.push(5);
    iqueue.push(7);

    cout << iqueue.size() << endl;
    cout << iqueue.front() << endl;

    iqueue.pop();
    cout << iqueue.front() << endl;
    iqueue.pop();
    cout << iqueue.front() << endl;
    iqueue.pop();
    cout << iqueue.front() << endl;
    cout << iqueue.size() << endl;
}