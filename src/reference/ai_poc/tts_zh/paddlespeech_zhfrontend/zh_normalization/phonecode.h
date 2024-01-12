#ifndef PHONECODE_H
#define PHONECODE_H

#include <regex>

using namespace std;

// #include <boost/xpressive/xpressive.hpp>
// using namespace boost::xpressive;

const regex RE_MOBILE_PHONE1_zh = regex(R"(((\+?86 ?)1([38]\d|5[0-35-9]|7[678]|9[89])\d{8})(?!\d))");
const regex RE_MOBILE_PHONE2_zh = regex(R"(((\+?86-?)?1([38]\d|5[0-35-9]|7[678]|9[89])\d{8})(?!\d))");

const regex RE_TELEPHONE_zh = regex(R"(((0(10|2[1-3]|[3-9]\d{2})-?)?[1-9]\d{7,8})(?!\d))");
// const sregex RE_MOBILE_PHONE = sregex::compile(R"((?<!\d)((\+?86 ?)?1([38]\d|5[0-35-9]|7[678]|9[89])\d{8})(?!\d))");
// const sregex RE_TELEPHONE = sregex::compile(R"((?<!\d)((0(10|2[1-3]|[3-9]\d{2})-?)?[1-9]\d{7,8})(?!\d))");
// // # 全国统一的号码400开头
const regex RE_NATIONAL_UNIFORM_NUMBER_zh = regex(R"((400)(-)?\d{3}(-)?\d{4})");
// const sregex RE_NATIONAL_UNIFORM_NUMBER = sregex::compile(R"((400)(-)?\d{3}(-)?\d{4})");

std::string replace_mobile_zh(smatch match);
std::string replace_phone_zh(smatch match);
std::string replace_mobile_zh_(smatch match);
// typedef std::string (*ppf)(boost::xpressive::smatch);  //此种方式最容易理解，定义了一个函数指针类型；函数名就是指针。
// std::string replace_phonecode(ppf p,std::string text,sregex e);
typedef std::string (*ppf)(smatch);  //此种方式最容易理解，定义了一个函数指针类型；函数名就是指针。
std::string replace_phonecode_zh(ppf p,std::string text,regex e);


#endif