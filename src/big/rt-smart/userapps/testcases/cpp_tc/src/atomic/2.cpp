#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

atomic_flag lock_stream = ATOMIC_FLAG_INIT;

void append_number_unlock(int x)
{
    int num;
    cout << "https://www";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << ".rt-";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << "thread";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << ".org/";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << setw(3) << setfill('0') << x << endl;
}

void append_number_lock(int x)
{
    while (lock_stream.test_and_set()) {
    }
    int num;
    cout << "https://www";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << ".rt-";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << "thread";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << ".org/";
    for (int i = 0; i < 1000000; i++)
        num++;
    cout << setw(3) << setfill('0') << x << endl;
    lock_stream.clear();
}

void string_split(const string &str, const char split, vector<string> &res)
{
    if (str == "")
        return;
    // 在字符串末尾也加入分隔符，方便截取最后一段
    string strs = str + split;
    size_t pos  = strs.find(split);

    // 若找不到内容则字符串搜索函数返回 npos
    while (pos != strs.npos) {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        // 去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos + 1, strs.size());
        pos  = strs.find(split);
    }
}

int main()
{
    cout << "un_lock" << endl;
    vector<thread> threads_unlock;
    for (int i = 1; i <= 20; ++i)
        threads_unlock.push_back(thread(append_number_unlock, i));
    for (auto &th : threads_unlock)
        th.join();
    cout << "lock" << endl;

    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    vector<thread> threads_lock;
    for (int i = 1; i <= 20; ++i)
        threads_lock.push_back(thread(append_number_lock, i));
    for (auto &th : threads_lock)
        th.join();

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    vector<string> str_list;
    string_split(s, '\n', str_list);
    bool b = true;
    for (auto str : str_list) {
        cout << str.length() << " ";
        if (str != "")
            if (str.length() != 29) {
                b = false;
            }
    }

    if (b)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
