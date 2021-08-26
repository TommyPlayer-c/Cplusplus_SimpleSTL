// file: 5hashset-test.cpp

#include <iostream>
#include "hash_set.h"
#include <cstring>

using namespace SimpleSTL;
using namespace std;

// 仿函数
struct eqstr {
    bool operator() (char* s1, char* s2) {
        return strcmp(s1, s2) == 0;
    }
};

void lookup(hash_set<char*, hash<char*>, eqstr>& Set, 
            char* word) {
    hash_set<char *, hash<char *>, eqstr>::iterator it = Set.find(word);
    cout << " " << word << ": " << (it != Set.end() ? "present" : "not present") << endl;
}

int main() {
    hash_set<char *, hash<char *>, eqstr> Set;
    Set.insert("kiwi");
    Set.insert("plum");
    Set.insert("apple");
    Set.insert("mango");
    Set.insert("apricot");
    Set.insert("banana");

    lookup(Set, "mango");
    lookup(Set, "apple");
    lookup(Set, "durian");

    auto ite1 = Set.begin();
    auto ite2 = Set.end();

    for (; ite1 != ite2; ++ite1)
        cout << *ite1 << ' ';
    cout << endl;

    hash_set<int> iSet;
    iSet.insert(59);
    iSet.insert(63);
    iSet.insert(108);
    iSet.insert(2);
    iSet.insert(53);
    iSet.insert(55);

    auto ite11 = iSet.begin();
    auto ite22 = iSet.end();
    // 为何已排序：
    // 因为bucket为193（内置28个质数中大于100的最小质数）
    // bucket足够多，造成排序假象
    for (; ite11 != ite22; ++ite11)
        cout << *ite11 << ' ';
    cout << endl;

    iSet.clear();
    iSet.insert(3);
    iSet.insert(196);
    iSet.insert(1);
    iSet.insert(389);
    iSet.insert(194);
    iSet.insert(387);
    ite11 = iSet.begin();
    for (; ite11 != ite22; ++ite11)
        cout << *ite11 << ' ';
    cout << endl;
}