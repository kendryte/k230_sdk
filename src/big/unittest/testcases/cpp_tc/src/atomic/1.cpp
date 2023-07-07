#include <atomic>
#include <iostream>
#include <sstream>

using namespace std;

string expect = R"(init atomic_flag_test 0
init atomic_bool_test1 0
init atomic_bool_test2 1
init atomic_int_test1 100
init atomic_int_test2 200
)";

int main(void)
{
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    atomic_flag atomic_flag_test = ATOMIC_FLAG_INIT;
    cout << "init atomic_flag_test " << atomic_flag_test.test_and_set() << endl;

    atomic_bool atomic_bool_test1;
    atomic_init(&atomic_bool_test1, false);
    cout << "init atomic_bool_test1 " << atomic_bool_test1 << endl;

    atomic<bool> atomic_bool_test2;
    atomic_init(&atomic_bool_test2, true);
    cout << "init atomic_bool_test2 " << atomic_bool_test2 << endl;

    atomic<int> atomic_int_test1(100);
    cout << "init atomic_int_test1 " << atomic_int_test1 << endl;

    atomic<int> atomic_int_test2;
    atomic_init(&atomic_int_test2, 200);
    cout << "init atomic_int_test2 " << atomic_int_test2 << endl;

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s == expect)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");
}
