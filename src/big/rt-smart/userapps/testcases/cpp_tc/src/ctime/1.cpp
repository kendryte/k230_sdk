#include <cmath>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <sstream>

using namespace std;

int main()
{
    int sleep_time = 3;
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    time_t now = time(nullptr);
    cout << "Now is: " << ctime(&now);

    time_t time1 = time(nullptr);
    sleep(sleep_time);
    time_t time2 = time(nullptr);

    double time_diff = difftime(time2, time1);
    cout << "time1: " << time1 << endl;
    cout << "time2: " << time2 << endl;
    cout << "time_diff: " << time_diff << "s" << endl;

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    if (s.length() > 0) {
        if (time_diff == sleep_time) {
            printf("{Test PASS}.\n");
        }
        else {
            printf("{Test FAIL}.\n");
        }
    }
    else {
        printf("{Test FAIL}.\n");
    }
}
