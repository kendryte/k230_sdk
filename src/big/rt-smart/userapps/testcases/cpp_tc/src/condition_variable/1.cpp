#include <condition_variable>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

using namespace std;

std::mutex              mtx;
std::condition_variable cv;
bool                    ready = false;

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

void print_id(int id)
{
    std::unique_lock<std::mutex> lck(mtx);
    while (!ready)
        cv.wait(lck);
    // ...
    std::cout << "thread " << id << '\n';
}

void go()
{
    std::unique_lock<std::mutex> lck(mtx);
    ready = true;
    cv.notify_all();
}

int main()
{
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    std::thread threads[10];
    // spawn 10 threads:
    for (int i = 0; i < 10; ++i)
        threads[i] = std::thread(print_id, i);

    std::cout << "10 threads ready to race...\n";
    go();  // go!

    for (auto &th : threads)
        th.join();

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    vector<string> str_list;
    string_split(s, '\n', str_list);

    if (str_list[0] == "10 threads ready to race...")
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
