#include "pinyin_utils.h"

void trim(string &s)
{
    //去掉首尾空格
    if( !s.empty() )
    {
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
    }
    //去掉字符串间的空格
    int index = 0;
    if( !s.empty())
    {
        while( (index = s.find(' ',index)) != string::npos)
        {
            s.erase(index,1);
        }
    }

}


// convert string to wstring
std::wstring to_wide_string(const std::string& input)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(input);
}
// convert wstring to string 
std::string to_byte_string(const std::wstring& input)
{
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(input);
}

// 开始分隔
vector<string> split(string s,char c)
{
    vector<string> results; 
    stringstream ss(s);
    string str;
     while (getline(ss, str, c)) {
        results.push_back(str);
    }
    return results;
}



// int main()
// {
//     string text = "导弹试验是在朝鲜和正恩与美国前总统唐纳德特朗普之间的峰会失败而陷入停滞之后进行的。";
//     trim(text);
//     cout << "Hello World";
//     return 0;
// }