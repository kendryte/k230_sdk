#ifndef CHRONOLOGY_H
#define CHRONOLOGY_H

#include <iostream>
#include <string>  
#include <sstream>
#include <map>
#include <vector>
#include <locale>
#include <assert.h>
#include <wchar.h>
#include <codecvt>



using namespace std;



const regex RE_DATE(R"((\d{4})年((0?[1-9]|1[0-2])月)?((((1|2)[0-9])|30|31|(0?[1-9]))([日号]))?)");
// # 用 / 或者 - 分隔的 YY/MM/DD 或者 YY-MM-DD 日期
const regex RE_DATE2(R"((\d{4})([- /.])(0?[1-9]|1[012])\2([12][0-9]|3[01]|0?[1-9]))");

// # 时刻表达式
const regex RE_TIME(R"(([0-1]?[0-9]|2[0-3]):([0-5][0-9])(:([0-5][0-9]))?)");

// 时间范围，如8:30-12:30
const regex RE_TIME_RANGE(R"(([0-1]?[0-9]|2[0-3]):([0-5][0-9])(:([0-5][0-9]))?(~|-)([0-1]?[0-9]|2[0-3]):([0-5][0-9])(:([0-5][0-9]))?)");

string replace_date(smatch match);
string replace_date2(smatch match);
string replace_time(smatch match);
#endif



