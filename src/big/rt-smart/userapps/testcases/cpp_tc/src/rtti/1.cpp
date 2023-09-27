#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace std;

string expect = R"(s
j
i
c
w
f
d
P1A
1A
P1B
P1C
1C
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

struct C
{
    void Print()
    {
        cout << "This is struct C." << endl;
    }
};

void test1()
{
    short    st  = 2;
    unsigned ui  = 10;
    int      i   = 10;
    char     ch  = 'a';
    wchar_t  wch = L'b';
    float    f   = 1.0f;
    double   d   = 2;

    cout << typeid(st).name() << endl;   // s
    cout << typeid(ui).name() << endl;   // j
    cout << typeid(i).name() << endl;    // i
    cout << typeid(ch).name() << endl;   // c
    cout << typeid(wch).name() << endl;  // w
    cout << typeid(f).name() << endl;    // f
    cout << typeid(d).name() << endl;    // d
}

void test2()
{
    A *pA1 = new A();
    A  a2;

    cout << typeid(pA1).name() << endl;  // P1A
    cout << typeid(a2).name() << endl;   // 1A

    B *pB1 = new B();
    cout << typeid(pB1).name() << endl;  // P1B

    C *pC1 = new C();
    C  c2;

    cout << typeid(pC1).name() << endl;  // P1C
    cout << typeid(c2).name() << endl;   // 1C
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
