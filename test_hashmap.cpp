// file: 5hashmap-test.cpp

#include <iostream>
#include "hash_map.h"
#include <cstring>

using namespace SimpleSTL;

struct eqstr {
    bool operator() (const char* s1, const char* s2) const {
        return strcmp(s1, s2) == 0;
    }
};

int main() {
    hash_map<char *, int, hash<char *>, eqstr> days;
    days["january"] = 31;
    days["february"] = 28;
    days["march"] = 31;
    days["april"] = 30;
    days["may"] = 31;
    days["june"] = 30;
    days["july"] = 31;
    days["august"] = 31;
    days["september"] = 30;
    days["october"] = 31;
    days["november"] = 30;
    days["december"] = 31;

    cout << "september -> " << days["september"] << endl;
    cout << "june -> " << days["june"] << endl;
    cout << "february -> " << days["february"] << endl;
    cout << "december -> " << days["december"] << endl;

    auto ite1 = days.begin();
    auto ite2 = days.end();
    for (; ite1 != ite2; ++ite1)
        std::cout << ite1->first << " ";
    cout << endl;
}