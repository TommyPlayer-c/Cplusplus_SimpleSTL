#include <iostream>
#include "./stack.h"
#include "./list.h"

using namespace std;
using namespace SimpleSTL;

int main() {
    stack<int> istack;
    istack.push(1);
    istack.push(3);
    istack.push(5);
    istack.push(7);

    cout << istack.size() << endl;
    cout << istack.top() << endl;

    istack.pop();
    cout << istack.top() << endl;
    istack.pop();
    cout << istack.top() << endl;
    istack.pop();
    cout << istack.top() << endl;
    cout << istack.size() << endl;
}