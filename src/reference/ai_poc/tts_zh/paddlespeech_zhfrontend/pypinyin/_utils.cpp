#include <regex>
#include <algorithm>
#include "constants.h"
#include "standard.h"
#include "phonetic_symbol.h"

string get_initials(string pinyin, bool strict) {
    if (strict) {
        for (auto i : _INITIALS) {
            if (pinyin.find(i) == 0) {
                return i;
            }
        }
    } else {
        for (auto i : _INITIALS_NOT_STRICT) {
            if (pinyin.find(i) == 0) {
                return i;
            }
        }
    }
    return "";
}

/*获取单个拼音中的韵母.
    :param pinyin: 单个拼音，无声调拼音
    :type pinyin: unicode
    :param strict: 是否严格遵照《汉语拼音方案》来处理声母和韵母
    :return: 韵母
    :rtype: unicode
*/
string get_finals(string pinyin, bool strict) {
    if (strict) {
        pinyin = convert_finals(pinyin);
    }
    string initials = get_initials(pinyin, strict);
    // 按声母分割，剩下的就是韵母
    string finals = pinyin.substr(initials.length());
    // 处理既没有声母也没有韵母的情况
    if (strict && std::find(_FINALS.begin(), _FINALS.end(), finals) == _FINALS.end()) {
        initials = get_initials(pinyin, false);
        finals = pinyin.substr(initials.size());
        if (std::find(_FINALS.begin(), _FINALS.end(), finals) != _FINALS.end()) {
            return finals;
        }
        return "";
    }
    if (finals.empty() && !strict) {
        return pinyin;
    }
    return finals;
}

vector<string> get_initials_finals(string pinyin, bool strict) {
    vector<string> result;
    if (strict) {
        pinyin = convert_finals(pinyin);
    }
    string initials = get_initials(pinyin, strict);
    result.push_back(initials);
    // 按声母分割，剩下的就是韵母
    string finals = pinyin.substr(initials.length());
    result.push_back(finals);
    // 处理既没有声母也没有韵母的情况
    if (strict && std::find(_FINALS.begin(), _FINALS.end(), finals) == _FINALS.end()) {
        result.clear();
        initials = get_initials(pinyin, false);
        finals = pinyin.substr(initials.size());
        result.push_back(initials);
        
        if (std::find(_FINALS.begin(), _FINALS.end(), finals) != _FINALS.end()) {
            result.push_back(finals);
            return result;
        }
        result.push_back("");
        return result;
    }
    if (finals.empty() && !strict) {
        return result;
    }
    return result;
}


string replace_symbol_to_number(string pinyin)
{
    smatch m;
    string value = pinyin;
    while (regex_search(value, m, RE_PHONETIC_SYMBOL))
    {
        string symbol = m[0];
        string to = PHONETIC_SYMBOL_DICT[symbol];
        value = regex_replace(value, RE_PHONETIC_SYMBOL, to,regex_constants::format_first_only);
    }
    for(auto &x:PHONETIC_SYMBOL_DICT_KEY_LENGTH_NOT_ONE)
    {
        value = regex_replace(value, regex(x.first), x.second);
    }
    return value;
}

string replace_symbol_to_no_symbol(string pinyin)
{
    string value = replace_symbol_to_number(pinyin);
    value = regex_replace(value, RE_NUMBER, "");
    return value;
}
bool has_finals(const string &pinyin) {
    // 鼻音: 'm̄', 'ḿ', 'm̀', 'ń', 'ň', 'ǹ ' 没有韵母
    for (auto symbol : {"m̄", "ḿ", "m̀", "ń", "ň", "ǹ"}) {
        if (pinyin.find(symbol) != string::npos) {
            return false;
        }
    }
    return true;
}

// int main()
// {
    
//     while (std::cin)
//    {
//         std::string pinyin;
//         std::getline(std::cin, pinyin);
//         // pinyin = get_initials(pinyin,true);
//         // cout<<pinyin<<endl;
//         // pinyin = get_finals(pinyin,false);
//         pinyin = replace_symbol_to_no_symbol(pinyin);
//         cout<<pinyin<<endl;
        
//    }
//     return 0;
// }