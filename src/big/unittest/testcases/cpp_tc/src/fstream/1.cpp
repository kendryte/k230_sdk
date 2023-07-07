#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

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

    string   ln;
    ifstream testFile("test.txt");
    if (testFile.is_open()) {
        while (getline(testFile, ln)) {
            cout << ln << '\n';
        }
        testFile.close();
    }
    else
        cout << "File is not there on the given path" << endl;

    ofstream creatMyFile("test.txt");
    creatMyFile << "Hello, C++ is a powerful language" << endl;
    creatMyFile.close();
    string   myText;
    ifstream readMyFile("test.txt");
    while (getline(readMyFile, myText)) {
        cout << myText;
    }
    readMyFile.close();

    cout.rdbuf(buffer);
    string s(ss.str());
    cout << s << endl;

    vector<string> str_list;
    string_split(s, '\n', str_list);
    bool b = false;
    for (auto str : str_list) {
        if (str == "Hello, C++ is a powerful language")
            b = true;
    }

    if (b)
        printf("{Test PASS}.\n");
    else
        printf("{Test FAIL}.\n");

    return 0;
}
