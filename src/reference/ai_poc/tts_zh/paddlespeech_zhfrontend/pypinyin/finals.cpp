#include <iostream>
#include <regex>
#include "_utils.h"
#include "standard.h"
#include "constants.h"

using namespace std;

string to_finals(string pinyin,bool strict){
    if(strict)
        pinyin = convert_finals(pinyin);
    bool has_fi = has_finals(pinyin);
    // # 替换声调字符为无声调字符
    pinyin = replace_symbol_to_no_symbol(pinyin);
    if(!has_fi)
        return pinyin;
    //获取韵母部分
    return get_finals(pinyin,false);
}

string to_finals_tone(string pinyin,bool strict){
    if(!has_finals(pinyin))
        return pinyin;
    //获取韵母部分
    return get_finals(pinyin,strict);
}

string to_finals_tone2(string pinyin,bool strict){
    if(strict)
        pinyin = convert_finals(pinyin);
    bool has_fi = has_finals(pinyin);

    //用数字表示声调
    pinyin = replace_symbol_to_number(pinyin);
    if(!has_fi)
        return pinyin;
    //获取韵母部分
    return get_finals(pinyin,false);
}

string to_finals_tone3(string pinyin,bool strict){
    if(strict)
        pinyin = convert_finals(pinyin);
    bool has_fi = has_finals(pinyin);

    //用数字表示声调
    pinyin = replace_symbol_to_number(pinyin);
    
    //将声调移动到最后
    pinyin = regex_replace(pinyin, RE_TONE3, "$1$3$2");
    if(!has_fi)
        return pinyin;
    //获取韵母部分
    return get_finals(pinyin,false);
}

vector<string> to_initials_finals_tone3(string pinyin,bool strict){

    if(strict)
        pinyin = convert_finals(pinyin);

    bool has_fi = has_finals(pinyin);

    //用数字表示声调
    pinyin = replace_symbol_to_number(pinyin);

    //将声调移动到最后
    pinyin = regex_replace(pinyin, RE_TONE3, "$1$3$2");

    if(!has_fi){
        vector<string> result;
        result.push_back(pinyin);
        result.push_back("");
        return result;
    }
    //获取韵母部分
    return get_initials_finals(pinyin,false);
}
