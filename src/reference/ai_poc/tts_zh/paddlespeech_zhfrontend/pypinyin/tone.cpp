#include <iostream>
#include <regex>
#include "_utils.h"
#include "constants.h"

using namespace std;
std::string to_tone(std::string pinyin, std::string tone) {
    return pinyin;
}

std::string to_tone2(std::string pinyin) {
    // 用数字表示声调
    pinyin = replace_symbol_to_number(pinyin);
    return pinyin;
}

string to_tone3(string pinyin){
    pinyin = to_tone2(pinyin);
    return regex_replace(pinyin, RE_TONE3, "$1$3$2");
}