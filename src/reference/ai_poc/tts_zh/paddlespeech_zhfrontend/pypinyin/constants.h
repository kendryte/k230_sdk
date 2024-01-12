#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <regex>

enum class Style {
    NORMAL,
    TONE,
    TONE2,
    TONE3,
    INITIALS,
    FIRST_LETTER,
    FINALS,
    FINALS_TONE,
    FINALS_TONE2,
    FINALS_TONE3,
    BOPOMOFO,
    BOPOMOFO_FIRST,
    CYRILLIC,
    CYRILLIC_FIRST,
    WADEGILES,
    SHENGMU_YUNMU,//分别返回声母韵母
};

// 声母表
const std::vector<std::string> _INITIALS = {
    "b",
    "p",
    "m",
    "f",
    "d",
    "t",
    "n",
    "l",
    "g",
    "k",
    "h",
    "j",
    "q",
    "x",
    "zh",
    "ch",
    "sh",
    "r",
    "z",
    "c",
    "s",
};
// 声母表, 把 y, w 也当作声母
const std::vector<std::string> _INITIALS_NOT_STRICT = {
    "b",
    "p",
    "m",
    "f",
    "d",
    "t",
    "n",
    "l",
    "g",
    "k",
    "h",
    "j",
    "q",
    "x",
    "zh",
    "ch",
    "sh",
    "r",
    "z",
    "c",
    "s",
    "y",
    "w",
};
// 韵母表
const std::vector<std::string> _FINALS = {
    "i",
    "u",
    "ü",
    "a",
    "ia",
    "ua",
    "o",
    "uo",
    "e",
    "ie",
    "üe",
    "ai",
    "uai",
    "ei",
    "uei",
    "ao",
    "iao",
    "ou",
    "iou",
    "an",
    "ian",
    "uan",
    "üan",
    "en",
    "in",
    "uen",
    "ün",
    "ang",
    "iang",
    "uang",
    "eng",
    "ing",
    "ueng",
    "ong",
    "iong",
    "er",
    "ê",
};

const std::regex RE_NUMBER("\\d");
const std::regex RE_PHONETIC_SYMBOL("ā|á|ǎ|à|ē|é|ě|è|ō|ó|ǒ|ò|ī|í|ǐ|ì|ū|ú|ǔ|ù|ü|ǖ|ǘ|ǚ|ǜ|ń|ň|ǹ|ḿ|ế|ề");

// 匹配使用数字标识声调的字符的正则表达式
const std::regex RE_TONE2("([aeoiuvnmê])([1-5])$");

// 匹配 TONE2 中标识韵母声调的正则表达式
const std::regex RE_TONE3("^([a-zê]+)([1-5])([a-zê]*)$");


#endif