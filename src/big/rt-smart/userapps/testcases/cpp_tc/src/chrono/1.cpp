#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <sstream>

using namespace std;
using namespace std::chrono;

string expect = R"(new era time:     Thu Jan  1 00:00:00 1970
new era time +1:  Fri Jan  2 00:00:00 1970
new era time +10: Sun Jan 11 00:00:00 1970
)";

int main()
{
    // 新纪元1970.1.1时间
    system_clock::time_point epoch;

    duration<int, ratio<60 * 60 * 24>> day(1);
    // 新纪元1970.1.1时间 + 1天
    system_clock::time_point ppt(day);

    using dday = duration<int, ratio<60 * 60 * 24>>;
    // 新纪元1970.1.1时间 + 10天
    time_point<system_clock, dday> t(dday(10));

    // 系统当前时间
    system_clock::time_point today = system_clock::now();

    // 转换为time_t时间类型
    time_t tm = system_clock::to_time_t(today);
    cout << "today:            " << ctime(&tm);

    time_t tm1 = system_clock::to_time_t(today + day);
    cout << "tomorrow:         " << ctime(&tm1);

    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    time_t tm2 = system_clock::to_time_t(epoch);
    cout << "new era time:     " << ctime(&tm2);

    time_t tm3 = system_clock::to_time_t(ppt);
    cout << "new era time +1:  " << ctime(&tm3);

    time_t tm4 = system_clock::to_time_t(t);
    cout << "new era time +10: " << ctime(&tm4);

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s == expect)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");
    return 0;
}
