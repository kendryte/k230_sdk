#include <iostream>
#include <sstream>
#include <string>

using namespace std;

string expect = R"(main:A constructor
)";

class A {
public:
    A(string s)
    {
        str.assign(s);
        cout << str << ":A constructor" << endl;
    }
    ~A()
    {
        cout << str << ":A destructor" << endl;
    }

private:
    string str;
};

A test1("global");

int main()
{
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    A test2("main");

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s == expect)  // 由于global的执行时刻早于main，析构晚于main。 因此只能捕获到main函数中的 A test2("main");
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    // 预期输出

    // global:A constructor
    // main:A constructor
    //
    // TEST PASS
    // main:A destructor
    // global:A destructor

    return 0;
}
