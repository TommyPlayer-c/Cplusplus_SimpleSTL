#include <iostream>
#include "./list.h"
#include "./vector.h"
#include "./memory.h"

using namespace std;
using namespace SimpleSTL;      

int main()
{
    vector<int> v(2, 9);
    cout << v[0] << " " << v[1] << " " << v[2] << endl;
    cout << v.size() << endl;
    cout << v.capacity() << endl;
    v.push_back(1034);
    cout << v.back() << endl;
    cout << v.size() << endl;
    cout << v.capacity() << endl;
}

