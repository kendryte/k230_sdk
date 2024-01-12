#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <regex>
#include <unordered_map>
#include "pinyin_utils.h"

using namespace std;



// u -> ü
std::map<std::string, std::string> UV_MAP = {
    {"u", "ü"},
    {"ū", "ǖ"},
    {"ú", "ǘ"},
    {"ǔ", "ǚ"},
    {"ù", "ǜ"},
};
std::set<std::string> U_TONES = {
    "u", "ū", "ú", "ǔ", "ù"
};
// ü行的韵跟声母j，q，x拼的时候，写成ju(居)，qu(区)，xu(虚)
regex UV_RE("^(j|q|x)(u|ū|ú|ǔ|ù)(.*)$");


std::set<std::string> I_TONES = {"i", "ī", "í", "ǐ", "ì"};
    
map<string, string> IU_MAP = {
    {"iu", "iou"},
    {"iū", "ioū"},
    {"iú", "ioú"},
    {"iǔ", "ioǔ"},
    {"iù", "ioù"},
};

set<string> IU_TONES = {"iu", "iū", "iú", "iǔ", "iù"};

regex IU_RE("^([a-z]+)(iu|iū|iú|iǔ|iù)$");   

map<string, string> UI_MAP = {
    {"ui", "uei"},
    {"uī", "ueī"},
    {"uí", "ueí"},
    {"uǐ", "ueǐ"},
    {"uì", "ueì"},
};

set<string> UI_TONES  = {"ui", "uī", "uí", "uǐ", "uì"};

regex UI_RE("^([a-z]+)(ui|uī|uí|uǐ|uì)$");  

// un -> uen
map<string, string> UN_MAP = {
    {"un","uen"},
    {"ūn","ūen"},
    {"ún","úen"},
    {"ǔn","ǔen"},
    {"ùn","ùen"},
};
set<string> UN_TONES = {"un","ūn","ún","ǔn","ùn"};
regex UN_RE("([a-z]+)(un|ūn|ún|ǔn|ùn)$");


std::string convert_zero_consonant(std::string pinyin) {
    // y: yu -> v, yi -> i, y -> i
    if (pinyin.find("y") == 0) {
        // 去除 y 后的拼音
        vector<int> size;
        StringArray sr =String2StringArray(pinyin,size);
        std::string no_y_py = pinyin.substr(size[0]);
        
        string first_char = no_y_py.length() > 0 ? sr[1] : "";

        // yu -> ü: yue -> üe
        if (U_TONES.find(first_char) != U_TONES.end()) {
            pinyin = UV_MAP.at(first_char) + pinyin.substr(size[0]+size[1]);
        // yi -> i: yi -> i
        } else if (I_TONES.find(first_char) != I_TONES.end()) {
            pinyin = no_y_py;
        // y -> i: ya -> ia
        } else {
            pinyin = "i" + no_y_py;
        }
        return "y"+pinyin;
    }

    // w: wu -> u, w -> u
    if (pinyin.find("w") == 0) {
        // 去除 w 后的拼音
        vector<int> size;
        StringArray sr= String2StringArray(pinyin,size);
        std::string no_w_py = pinyin.substr(size[0]);
        string first_char = no_w_py.length() > 0 ? sr[1] : "";

        // wu -> u: wu -> u
        if (U_TONES.find(first_char) != U_TONES.end()) {
            pinyin = pinyin.substr(size[0]);
        // w -> u: wa -> ua
        } else {
            pinyin = "u" + pinyin.substr(size[0]);
        }

        return "w" +pinyin;
    }

    return pinyin;
};

std::string convert_uv(const std::string &pinyin) {
    /*
    ü 转换，还原原始的韵母

    ü行的韵跟声母j，q，x拼的时候，写成ju(居)，qu(区)，xu(虚)，
    ü上两点也省略；但是跟声母n，l拼的时候，仍然写成nü(女)，lü(吕)。
    */
    smatch m;
    if (regex_match(pinyin, m, UV_RE)) {
        return m[1].str() + UV_MAP[m[2].str()] + m[3].str();
    }
    else
        return pinyin;
};
string convert_iou(string pinyin) {
    /*iou 转换，还原原始的韵母
    iou，uei，uen前面加声母的时候，写成iu，ui，un。
    例如niu(牛)，gui(归)，lun(论)。
    */
    smatch m;
    if (regex_match(pinyin, m, IU_RE)) {
        return m[1].str() + IU_MAP[m[2].str()];
    }
    else
        return pinyin;
};

string convert_uei(string pinyin) {
    /*iou 转换，还原原始的韵母
    iou，uei，uen前面加声母的时候，写成iu，ui，un。
    例如niu(牛)，gui(归)，lun(论)。
    */
    smatch m;
    if (regex_match(pinyin, m, UI_RE)) {
        return m[1].str() + UI_MAP[m[2].str()];
    }
    else
        return pinyin;
};
string convert_uen(string pinyin) {
    /*iou 转换，还原原始的韵母
    iou，uei，uen前面加声母的时候，写成iu，ui，un。
    例如niu(牛)，gui(归)，lun(论)。
    */
    smatch m;
    if (regex_match(pinyin, m, UN_RE)) {
        return m[1].str() + UN_MAP[m[2].str()];
    }
    else
        return pinyin;
};

std::string convert_finals(std::string &pinyin){
    pinyin = convert_zero_consonant(pinyin);

    pinyin = convert_uv(pinyin);
    pinyin = convert_iou(pinyin);
    pinyin = convert_uei(pinyin);
    pinyin = convert_uen(pinyin);
    return pinyin;
}

// int main()
// {
//     // string pinyin = "yu";
//     // string pinyin = "wu";
//     while (std::cin)
//    {
//         std::string pinyin;
//         std::getline(std::cin, pinyin);
//         pinyin = convert_finals(pinyin);
//         cout<<pinyin<<endl;
//    }
//     return 0;
// }