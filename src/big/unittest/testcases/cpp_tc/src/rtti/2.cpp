#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;

string expect = R"(P1A
1A
P1C
1D
)";

class A {
public:
    void Print()
    {
        cout << "This is class A." << endl;
    }
};

class B : public A {
public:
    void Print()
    {
        cout << "This is class B." << endl;
    }
};

class C {
public:
    virtual void Print()
    {
        cout << "This is class C." << endl;
    }
};

class D : public C {
public:
    void Print()
    {
        cout << "This is class D." << endl;
    }
};

int main()
{
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    A *pA = new B();
    cout << typeid(pA).name() << endl;   // P1A
    cout << typeid(*pA).name() << endl;  // 1A

    C *pC = new D();
    cout << typeid(pC).name() << endl;   // P1C
    cout << typeid(*pC).name() << endl;  // 1D

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s == expect)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
