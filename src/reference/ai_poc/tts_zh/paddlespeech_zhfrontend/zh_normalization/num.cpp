/*
Rules to verbalize numbers into Chinese characters.
https://zh.wikipedia.org/wiki/中文数字#現代中文
*/

#include "num.h"
#include "pinyin_utils.h"



using namespace std;


unordered_map<char,string>DIGITS={{'0',"零"},{'1',"一"},{'2',"二"},{'3',"三"},
{'4',"四"},{'5',"五"},{'6',"六"},{'7',"七"},{'8',"八"},{'9',"九"}};


unordered_map<int,string>UNITS={{1,"十"},{2,"百"},{3,"千"},{4,"万"},{8,"亿"}};


vector<string> _get_value(string value_string, bool use_zero = true) {
    string stripped = value_string;
    while (stripped.size() > 0 && stripped[0] == '0') {
        stripped.erase(stripped.begin());
    }
    if (stripped.size() == 0) {
        return {};
    } else if (stripped.size() == 1) {
        if (use_zero && stripped.size() < value_string.size()) {
            return {DIGITS['0'], DIGITS[stripped[0]]};
        } else {
            return {DIGITS[stripped[0]]};
        }
    } else {
        int largest_unit = 0;
        if(stripped.size()>8)
        {
            largest_unit = 8;
        }
        else if(stripped.size()>4)
        {
            largest_unit = 4;
        }
        else if(stripped.size()>3)
        {
            largest_unit = 3;
        }
        else if(stripped.size()>2)
        {
            largest_unit = 2;
        }
        else if(stripped.size()>1)
        {
            largest_unit = 1;
        }
    
    
        string first_part = value_string.substr(0, value_string.size() - largest_unit);
        string second_part = value_string.substr(value_string.size() - largest_unit);
        vector<std::string> result = _get_value(first_part);
        if((result[0]=="二")&&(largest_unit>1)){
            result[0]="两";
        }
        result.push_back(UNITS[largest_unit]);
        vector<std::string> second_result = _get_value(second_part);
        result.insert(result.end(), second_result.begin(), second_result.end());
        return result;
    }
}

string verbalize_cardinal(const string& value_string) {
    if (value_string.empty()) {
        return "";
    }

    // 000 -> '零' , 0 -> '零'
    string value_string_trimmed = value_string;
    value_string_trimmed.erase(0, value_string_trimmed.find_first_not_of('0'));
    if (value_string_trimmed.empty()) {
        return DIGITS['0'];
    }    
    
    vector<string> result_symbols = _get_value(value_string_trimmed);
    
    // verbalized number starting with '一十*' is abbreviated as `十*`
    if (result_symbols.size() >= 2 && result_symbols[0] == DIGITS['1'] && result_symbols[1] == UNITS[1]) {
        result_symbols.erase(result_symbols.begin());
    }
    string results;
    for (auto digit : result_symbols) {
        results +=digit;
    }

    return results;
}

string verbalize_digit(string value_string,bool alt_one/*=false*/)
{
    string result="";
    for(auto ch:value_string)
    {

        result+=DIGITS[ch];
        cout<<"result "<<endl;
    }
    
    return result;
}

string num2str(string value_string) {

    vector<string> integer_decimal = split(value_string, '.');
   
    string integer;
    string decimal;
    if (integer_decimal.size() == 1) {
        integer = integer_decimal[0];
        decimal = "";
    } else if (integer_decimal.size() == 2) {
        integer = integer_decimal[0];
        decimal = integer_decimal[1];
    } else {
        throw invalid_argument(
            "The value string: '" + value_string + "' has more than one point in it."
        );
    }

   

    string result = verbalize_cardinal(integer);

    
    decimal = decimal.substr(0, decimal.find_last_not_of('0') + 1);   
    
    if (!decimal.empty()) {
        // '.22' is verbalized as '零点二二'
        // '3.20' is verbalized as '三点二
        result = result.empty() ? "零" : result;
        result += "点" + verbalize_digit(decimal);
    }

    return result;
}




string replace_frac(smatch match)
{
    string sign="";
    if(match[1]=='-')
   {
        sign="负";
   }
   
    string nominator = match[2].str();
    string denominator = match[3].str();
    nominator = num2str(nominator);
    denominator = num2str(denominator);
    string result = sign+denominator+"分之"+nominator;
    
    return result;
}




string replace_percentage(smatch match)
{
    string sign="";
    if(match[1]=='-')
   {
        sign="负";
   }
   
    string percent = match[2].str();
    percent = num2str(percent);
    string result = sign+"百分之"+percent;
    
    return result;
}




string replace_negative_num(smatch match)
{
    string sign="";
    if(match[1]=='-')
   {
        sign="负";
   }
   
    string number = match[2].str();
    number = num2str(number);
    string result = sign+number;
    
    return result;
}




string replace_default_num(smatch match)
{
    string number = match[0];
    
    return verbalize_digit(number);
}



string replace_positive_quantifier(smatch match) {
    string wnumber = match[1];
    string match_2 = match[2];
    
    if (match_2 == "+") {
        match_2 = "多";
    }
    if (match_2.empty()) {
        match_2 = "";
    }
    string quantifiers = match[3];
    string number = num2str((wnumber));
    
    string result = number + (match_2) + (quantifiers);
    return result;
}

string replace_number(smatch match)
{
    string sign=match[1].str();
    string number = match[2].str();
    string pure_decimal = match[5].str();
    string result="";
    if(!pure_decimal.empty())
    {
        result=num2str(pure_decimal);
    }
    else
    {
        result = num2str(number);
        if(!sign.empty())
            result = "负" + result;
    }
   
    return result;
}

//查找并替换
string replace(pf p,string text,regex e){
    string s = text;
    string result="";
    smatch m;
    while (regex_search(s, m, e)) {
        
        result+=m.prefix(); 
        //替换
        string replace_str = p(m);
        result+=replace_str;
       
        s = m.suffix().str(); 
        // 返回末端，作为新的搜索的开始
    }
    if(!s.empty())
        result+=s;
    return result;
}

// //查找并替换
// string wreplace(wpf p,string text,wregex e){
//     string result="";
    
//     wstring s = to_wide_string(text);
    
//     wsmatch m;
//     while (regex_search(s, m, e)) {
//         result+=to_byte_string(m.prefix());
//         // 替换
//         string replace_str = p(m);
//         result+=replace_str;
//         s = m.suffix().str(); // 返回末端，作为新的搜索的开始
        
         
//     }
//     if(!s.empty())
//         result+=to_byte_string(s);
    
//     return result;
// }


string replace_range(smatch match)
{
    string first = match[1];
    string second = match[8];
    cout<<"first"<<endl;
    cout<<"second"<<second<<endl;
    first = replace(replace_number,first,RE_NUMBER_);
    second = replace(replace_number,second,RE_NUMBER_);
    return first + "到" + second;
}















// int main()
// {
    
//     string text = "-20/30和55/66，5%。-6%。00034,345 00010000 20~0.5 20多朵花 30多岁";
//     text = replace(replace_frac,text,RE_FRAC);
//     text = replace(replace_percentage,text,RE_PERCENTAGE);
//     text = replace(replace_negative_num,text,RE_INTEGER);
//     text = replace(replace_default_num,text,RE_DEFAULT_NUM);
//     text = replace(replace_range,text,RE_RANGE);
//     text = replace(replace_positive_quantifier,text,RE_POSITIVE_QUANTIFIERS);
    
//     cout<<text<<endl;
   
//     return 0;
// }
