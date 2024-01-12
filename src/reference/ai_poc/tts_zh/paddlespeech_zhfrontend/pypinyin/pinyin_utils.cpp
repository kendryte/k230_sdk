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
//去掉首尾空格
void trim_shouwei(string &s)
{
    //去掉首尾空格
    if( !s.empty() )
    {
        s.erase(0,s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
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
//ord ,获取字符对应的unicode值
int ord(const std::string& str, std::vector<int64_t>& codes)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (!(str[i] & 0x80))
        { // 0xxxxxxx
          // 7bit, total 7bit
          //rp.rune = (uint8_t) (str[0]) & 0x7f;
          //rp.len = 1;
            int64_t code = (uint8_t) (str[i]) & 0x7f;
            codes.push_back(code);
        }
        else if ((uint8_t) str[i] <= 0xdf)
        {
            if ((i + 1) < str.size())
            {
                // 110xxxxxx
                // 5bit, total 5bit
                int64_t code = (uint8_t) (str[i]) & 0x1f;
                // 6bit, total 11bit
                code <<= 6;
                code |= (uint8_t) (str[i + 1]) & 0x3f;
                codes.push_back(code);
                i++;
            }
            else
            {
                printf("Invalid utf8 string 1\n");
                return -1;
            }
        }
        else if ((uint8_t) str[i] <= 0xef)
        {
            if ((i + 2) < str.size())
            {
                // 1110xxxxxx
                // 4bit, total 4bit
                int64_t code = (uint8_t) (str[i]) & 0x0f;
                // 6bit, total 10bit
                code <<= 6;
                code |= (uint8_t) (str[i + 1]) & 0x3f;
                // 6bit, total 16bit
                code <<= 6;
                code |= (uint8_t) (str[i + 2]) & 0x3f;
                codes.push_back(code);
                i += 2;
            }
            else
            {
                printf("Invalid utf8 string 2\n");
                return -1;
            }
        }
        else if ((uint8_t) str[i] <= 0xf7)
        {
            if ((i + 3) < str.size())
            {
                // 11110xxxx
                // 3bit, total 3bit
                int64_t code = (uint8_t) (str[i]) & 0x07;
                // 6bit, total 9bit
                code <<= 6;
                code |= (uint8_t) (str[i + 1]) & 0x3f;
                // 6bit, total 15bit
                code <<= 6;
                code |= (uint8_t) (str[i + 2]) & 0x3f;
                // 6bit, total 21bit
                code <<= 6;
                code |= (uint8_t) (str[i + 3]) & 0x3f;
                codes.push_back(code);
                i += 3;
            }
            else
            {
                printf("Invalid utf8 string 3\n");
                return -1;
            }
        }
        else
        {
            printf("Invalid utf8 string 4\n");
            return -1;
        }
    }
    return 0;
}
/*
将string转成每个字符为string的array数组
一字节：0*******
两字节：110*****，10******
三字节：1110****，10******，10******
四字节：11110，10******，10******，10******
五字节：111110，10******，10******，10******，10******
六字节：1111110，10******，10******，10******，10******，10******
*/
StringArray String2StringArray(const std::string& str,std::vector<int>& size)
{
    StringArray sr;
    //判断每个字符由几个字节组成
    for (size_t i = 0; i < str.size();i)
    {
        if (!((uint8_t)str[i] & 0x80))//一个字节
        {
            sr.push_back(str.substr(i,1));
            i++;
            size.push_back(1);
        }
        else if(((uint8_t)str[i] >>5)==0x6)//2个字节
        {
            sr.push_back(str.substr(i,2));
            i+=2;
            size.push_back(2);
        }
        else if(((uint8_t)str[i] >>4)==0xe)//3个字节
        {
            sr.push_back(str.substr(i,3));
            i+=3;
            size.push_back(3);
        }
        else if(((uint8_t)str[i] >>3)==0x1e)//4个字节
        {
            sr.push_back(str.substr(i,4));
            i+=4;
            size.push_back(4);
        }
        else if(((uint8_t)str[i] >>2)==0x3e)//5个字节
        {
            sr.push_back(str.substr(i,5));
            i+=5;
            size.push_back(5);
        }
        else if(((uint8_t)str[i] >>1)==0x7e)//6个字节
        {
            sr.push_back(str.substr(i,6));
            i+=6;
            size.push_back(6);
        }
    }
    return sr;
}




// int main()
// {
//     string text = "导弹试验是在朝鲜和正恩与美国前总统唐纳德特朗普之间的峰会失败而陷入停滞之后进行的。";
//     trim(text);
//     cout << "Hello World";
//     return 0;
// }