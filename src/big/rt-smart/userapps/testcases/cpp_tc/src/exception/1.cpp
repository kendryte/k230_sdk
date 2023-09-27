#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

string expect = R"(before dividing.
catch(int) -1
finished
vector::_M_range_check: __n (which is 100) >= this->size() (which is 10)
basic_string::at: __n (which is 100) >= this->size() (which is 5)
std::bad_cast
)";

class Base {
    virtual void func() {}
};

class Derived : public Base {
public:
    void Print() {}
};

int main()
{
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    double m = 1, n = 0;
    try {
        cout << "before dividing." << endl;
        if (n == 0)
            throw -1;  // 抛出int类型异常
        else
            cout << m / n << endl;
        cout << "after dividing." << endl;
    }
    catch (double d) {
        cout << "catch(double) " << d << endl;
    }
    catch (int e) {
        cout << "catch(int) " << e << endl;
    }
    cout << "finished" << endl;

    vector<int> v(10);
    try {
        v.at(100) = 100;  // 拋出 out_of_range 异常
    }
    catch (out_of_range &e) {
        cout << e.what() << endl;
    }

    string str = "hello";
    try {
        char c = str.at(100);  // 拋出 out_of_range 异常
    }
    catch (out_of_range &e) {
        cout << e.what() << endl;
    }

    Base b;
    try {
        Derived &rd = dynamic_cast<Derived &>(b);  // 此转换若不安全，会拋出 bad_cast 异常
        rd.Print();
    }
    catch (bad_cast &e) {
        cout << e.what() << endl;
    }

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s == expect)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
