// filename: 4vector-test.cpp

#include "vector.h"
#include <iostream>
#include <algorithm>

using namespace SimpleSTL;
using namespace std;

void printVector(vector<int> iv) {
    for (int i = 0; i < iv.size(); i++) {
        cout << iv[i] << " ";
    }
    cout << endl;
}

int main() {
    int i;
    vector<int> iv(2, 9);
    cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;

    for (int i = 1; i < 5; i++) {
        iv.push_back(i);
        cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;
    }

    printVector(iv);

    iv.push_back(i);
    cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;
    printVector(iv);

    iv.pop_back();
    iv.pop_back();
    cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;

    iv.pop_back();
    cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;

    vector<int>::iterator ivite = find(iv.begin(), iv.end(), 1);
    if (ivite!=iv.end())
        iv.erase(ivite);
    cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;
    printVector(iv);

    ivite = find(iv.begin(), iv.end(), 2);
    if (ivite!=iv.end())
        iv.insert(ivite, 3, 7);
    cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;
    printVector(iv);

    iv.clear();
    cout << "size=" << iv.size() << endl << "capacity=" << iv.capacity() << endl;
}