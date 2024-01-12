#ifndef TEXT_NORMALIZATION_H
#define TEXT_NORMALIZATION_H
#include <iostream>
#include <string>  
#include <sstream>
#include <map>
#include <vector>
#include <locale>
#include <assert.h>
#include <wchar.h>
#include <codecvt>
#include <regex>

using namespace std;
std::vector<std::string> normalize(std::string text);

std::vector<long int> normalize_sentence_(std::string sentence);
std::string normalize_sentence(std::string sentence);

#endif