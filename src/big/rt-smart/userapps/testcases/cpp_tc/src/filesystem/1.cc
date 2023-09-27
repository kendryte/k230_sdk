#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;
using namespace std::filesystem;

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
    stringstream ss;
    streambuf   *buffer = cout.rdbuf();
    cout.rdbuf(ss.rdbuf());

    path str("./");
    if (!exists(str))  // 必须先检测目录是否存在才能使用文件入口.
        return 1;
    directory_entry entry(str);                         // 文件入口
    if (entry.status().type() == file_type::directory)  // 这里用了C++11的强枚举类型
        cout << "is dir" << endl;
    directory_iterator list(str);  // 文件入口容器
    for (auto &it : list)
        cout << it.path().filename() << endl;  // 通过文件入口（it）获取path对象，再得到path对象的文件名，将之输出

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    vector<string> str_list;
    string_split(s, '\n', str_list);

    if (str_list[0] == "is dir" && str_list.size() > 1)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
