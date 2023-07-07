#include <iostream>
#include <random>
#include <sstream>

using namespace std;

string expect = R"(0
1
7
4
5
2
0
6
6
9
0.519416
0.0345721
0.5297
0.00769819
0.0668422
0.686773
0.930436
0.526929
0.653919
0.701191
0
1
1
0
1
0
0
1
0
1
)";

int main()
{
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    default_random_engine              e;
    uniform_int_distribution<unsigned> u1(0, 9);
    for (int i = 0; i < 10; ++i)
        cout << u1(e) << endl;

    uniform_real_distribution<double> u2(0.0, 1.0);
    for (int i = 0; i < 10; ++i)
        cout << u2(e) << endl;

    bernoulli_distribution u3;
    for (int i = 0; i < 10; ++i)
        cout << u3(e) << endl;

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s == expect)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
