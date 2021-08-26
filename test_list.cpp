#include <iostream>
#include "./list.h"
#include "./vector.h"
#include "./memory.h"

using namespace std;
using namespace SimpleSTL;     
  
//从前向后显示list队列的全部元素   
void print_list(const list<int> list, char *name)   
{   
    SimpleSTL::list<int>::iterator plist;   
      
    cout << "The contents of " << name << " : ";   
    for(plist = list.begin(); plist != list.end(); plist++)   
        cout << *plist << " ";   
    cout << endl;   
}   

int main()
{
    list<int> l1( {1,2,3,8,9} );
    list<int> l2( {0,4,5,6,7} );
    print_list(l1, "l1");
    print_list(l2, "l2");
    l1.merge(l2);
    print_list(l1, "l1");

    vector<int> v(1000, -9);
    cout << v[997] << " " << v[998] << " " << v[999] << endl;
    cout << v.size() << endl;
    cout << v.capacity() << endl;
    v.push_back(1034);
    cout << v.back() << endl;
    cout << v.size() << endl;
    cout << v.capacity() << endl;
}

