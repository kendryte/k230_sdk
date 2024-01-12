#ifndef _UTILS_H
#define _UTILS_H

#include <iostream>
using namespace std;

std::string replace_symbol_to_number(std::string pinyin);
string replace_symbol_to_no_symbol(string pinyin);
string get_initials(string pinyin, bool strict);
string get_finals(string pinyin, bool strict);
vector<string> get_initials_finals(string pinyin, bool strict);
bool has_finals(const string &pinyin);

#endif