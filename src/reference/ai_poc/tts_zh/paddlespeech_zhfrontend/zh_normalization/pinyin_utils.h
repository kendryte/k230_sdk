#ifndef PINYIN_UTILS_H
#define PINYIN_UTILS_H

#include <iostream>
#include <string>  
#include <sstream>
#include <vector>
#include <locale>
#include <codecvt>
using namespace std;

void trim(string &s);

// convert string to wstring
std::wstring to_wide_string(const std::string& input);

// convert wstring to string 
std::string to_byte_string(const std::wstring& input);

vector<string> split(string s,char c);

#endif
