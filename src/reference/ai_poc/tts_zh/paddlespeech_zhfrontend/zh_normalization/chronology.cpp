
#include "num.h"
#include "pinyin_utils.h"
#include <regex>
#include "chronology.h"





string _time_num2str(string num_string) {
    // """A special case for verbalizing number in time."""
    
    string result = num2str(num_string.substr(num_string.find_first_not_of('0')));
    if (num_string.at(0) == '0') {
        result = DIGITS['0'] + result;
        
    }
    return result;
}

string replace_time(smatch match) {
    bool is_range = match.size() > 5;

    string hour = match[1];
    string minute = match[2];
    string second = match[4];

    string hour_2;
    string minute_2;
    string second_2;

    if (is_range) {
        hour_2 = match[6];
        minute_2 = match[7];
        second_2 = match[9];
    }

    string result = num2str(hour)+"点";
    if (minute.find_first_not_of('0')!=string::npos) {
        if (stoi(minute) == 30) {
            result += "半";
        } else {
            result += _time_num2str(minute)+"分";
        }
    }
    if ((!second.empty()) && (second.find_first_not_of('0')!=string::npos)) {
        result += _time_num2str(second)+"秒";
    }

    if (is_range) {
        result += "至";
        result += num2str(hour_2)+"点";
        if (minute_2.find_first_not_of('0')!=string::npos) {
            if (stoi(minute_2) == 30) {
                result += "半";
            } else {
                result += _time_num2str(minute_2)+"分";
            }
        }
        if ((!second_2.empty()) && (second_2.find_first_not_of('0')!=string::npos)) {
            result += _time_num2str(second_2)+"秒";
        }
    }

    return result;
}


string replace_date(smatch match) {
    string result;
    if (match[1].matched) {
        result += verbalize_digit((match[1].str())) + "年";
    }
    if (match[3].matched) {
        result += verbalize_cardinal((match[3].str())) + "月";
    }
    if (match[5].matched) {
        result += verbalize_cardinal((match[5].str())) + (match[9].str());
    }
    return result;
}


string replace_date2(smatch match) {
    string result;
   
    if (match[1].matched) {
        result += verbalize_digit(match[1].str()) + "年";
    }
    if (match[3].matched) {
        result += verbalize_cardinal(match[3].str()) + "月";
    }
    if (match[4].matched) {
        result += verbalize_cardinal(match[4].str()) + "日";
    }
    return result;
}

// int main()
// {
    
//     string text = "-20/30和55/66，5%。-6%。00034,345 00010000 20~0.5 20+朵花 30多岁 2:15:30  4:20:10-5:16:10 2020年3月4日 2010/03/04  2250-01-03";
    
//     text = replace(replace_time,text,RE_TIME_RANGE);
//     text = replace(replace_time,text,RE_TIME);
//     text = replace(replace_date,text,RE_DATE);
//     text = replace(replace_date2,text,RE_DATE2);
//     text = replace(replace_positive_quantifier,text,RE_POSITIVE_QUANTIFIERS);
//     cout<<text<<endl;
   
//     return 0;
// }


