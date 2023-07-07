#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;

string expect = R"(I am a B truly.
I am a C truly.
I am a B truly.
I am a C truly.
)";

class A {
public:
    virtual void Print()
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

class C : public A {
public:
    void Print()
    {
        cout << "This is class C." << endl;
    }
};

void Handle(A *a)
{
    if (typeid(*a) == typeid(A)) {
        cout << "I am a A truly." << endl;
    }
    else if (typeid(*a) == typeid(B)) {
        cout << "I am a B truly." << endl;
    }
    else if (typeid(*a) == typeid(C)) {
        cout << "I am a C truly." << endl;
    }
    else {
        cout << "I am alone." << endl;
    }
}

void Handle1(A *a)
{
    if (dynamic_cast<B *>(a)) {
        cout << "I am a B truly." << endl;
    }
    else if (dynamic_cast<C *>(a)) {
        cout << "I am a C truly." << endl;
    }
    else {
        cout << "I am alone." << endl;
    }
}

void test1()
{
    A *pA = new B();
    Handle(pA);
    delete pA;
    pA = new C();
    Handle(pA);
    delete pA;
}

void test2()
{
    A *pA = new B();
    Handle1(pA);
    delete pA;
    pA = new C();
    Handle1(pA);
    delete pA;
}

int main()
{
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    test1();
    test2();

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s == expect)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
