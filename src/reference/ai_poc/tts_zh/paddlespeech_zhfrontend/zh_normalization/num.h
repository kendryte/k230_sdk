#ifndef NUM_H
#define NUM_H
#include <iostream>
#include <string>  
#include <sstream>
#include <unordered_map>
#include <vector>
#include <locale>
#include <assert.h>
#include <wchar.h>
#include <codecvt>
#include <regex>


using namespace std;

extern unordered_map<char,string>DIGITS;
// const string COM_QUANTIFIERS = "()";


// # 数字表达式

// # 纯小数
const std::regex RE_DECIMAL_NUM(R"((-?)((\d+)(\.\d+))|(\.(\d+)))");

// # 编号-无符号整形
// # 00078
const std::regex RE_DEFAULT_NUM(R"(\d{3}\d*)");

// 分数表达式
const std::regex RE_FRAC(R"((-?)(\d+)/(\d+))");
//加号表达式
const std::regex RE_Plus(R"((\d+)(\+)(\d+))");
//比值表达式
const std::regex RE_Ratio(R"((\d+)(:)(\d+))");

// 整数表达式
// 带负号的整数 -10
const std::regex RE_INTEGER(R"((-)(\d+))");

const std::regex RE_NUMBER_(R"((-?)((\d+)(\.\d+)?)|(\\.(\d+)))");

//百分数表达式
const std::regex RE_PERCENTAGE(R"((-?)(\d+(\.\d+)?)%)");

const std::regex RE_RANGE(R"(((-?)((\d+)(\.\d+)?)|(\.(\d+)))[-~]((-?)((\d+)(\.\d+)?)|(\.(\d+))))");


// 正整数 + 量词
// const std::regex RE_POSITIVE_QUANTIFIERS(R"((\d+)([多|余|几|\+])?)" + COM_QUANTIFIERS );
const std::regex RE_POSITIVE_QUANTIFIERS(R"((\d+)(多|余|几|所|朵|匹|张|座|回|场|尾|条|个|首|阙|阵|网|炮|顶|丘|棵|只|支|袭|辆|挑|担|颗|壳|窠|曲|墙|群|腔|砣|座|客|贯|扎|捆|刀|令|打|手|罗|坡|山|岭|江|溪|钟|队|单|双|对|出|口|头|脚|板|跳|枝|件|贴|针|线|管|名|位|身|堂|课|本|页|家|户|层|丝|毫|厘|分|钱|两|斤|担|铢|石|钧|锱|忽|(千|毫|微)克|毫|厘|(公)分|分|寸|尺|丈|里|寻|常|铺|程|(千|分|厘|毫|微)米|米|撮|勺|合|升|斗|石|盘|碗|碟|叠|桶|笼|盆|盒|杯|钟|斛|锅|簋|篮|盘|桶|罐|瓶|壶|卮|盏|箩|箱|煲|啖|袋|钵|年|月|日|季|刻|时|周|天|秒|分|小时|旬|纪|岁|世|更|夜|春|夏|秋|冬|代|伏|辈|丸|泡|粒|颗|幢|堆|条|根|支|道|面|片|张|颗|块|元|(亿|千万|百万|万|千|百)|(亿|千万|百万|万|千|百|美|)元|(亿|千万|百万|万|千|百|)块|角|毛|分|\+)?)");


string replace_commas(string text);
string replace_default_num(std::smatch match);
string replace_frac(std::smatch match);
string replace_negative_num(std::smatch match);
string replace_number(std::smatch match);
string replace_percentage(std::smatch match);
string replace_positive_quantifier(std::smatch match);
string replace_range(std::smatch match);


string num2str(string value_string);
string verbalize_cardinal(const string& value_string) ;
string verbalize_digit(string value_string,bool alt_one=false);
typedef string (*pf)(std::smatch);  //此种方式最容易理解，定义了一个函数指针类型；函数名就是指针。
// typedef string (*wpf)(std::wsmatch);  //此种方式最容易理解，定义了一个函数指针类型；函数名就是指针。
string replace(pf p,string text,std::regex e);
// string wreplace(wpf p,string text,std::wregex e);
#endif
