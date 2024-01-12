#include <iostream>
#include <string>  
#include <sstream>
#include <algorithm>  
#include <map>
#include <vector>
#include <regex>
#include <locale>
#include <codecvt>
#include "pinyin_utils.h"

#include "char_convert.h"
#include "chronology.h"
#include "constants.h"
#include "num.h"
#include "phonecode.h"
#include "quantifier.h"
#include "constants.h"
#include <chrono>

using namespace std;


vector<string> _split(string text,string lang="zh")
{
    vector<string> text_list;
    //去掉所有空格
    trim(text);

    wstring wtext = to_wide_string(text);
    std::wstring filtered_str;
    // Only for pure Chinese here
    if(lang == "zh")
    { 
        // 过滤掉特殊字符,并将标点符号替换为\n
        
        for (wchar_t c : wtext) {
          
          if((c==L'：') | (c==L'、') | (c==L'，')| (c==L'；') | (c==L'。') | (c== L'？')|(c==L'！') | (c==L',') | (c==L';') | (c==L'?') | (c==L'!') | (c==L'”') | (c==L'’'))
          {
            text = to_byte_string(filtered_str)+'\n';  
            text_list.push_back(text);
            filtered_str = L"";
            continue;
          }
          if (c != L'《' && c != L'》' && c != L'【' && c != L'】' && c != L'（' && c != L'）' && c != L'“' && c != L'”' && c != L'…'&& c != L'\\') {
            filtered_str += c;
          }
        }
        if(filtered_str !=L"")
            text_list.push_back(to_byte_string(filtered_str));


    }

    // for(auto t:text_list)
    // {
    //     cout<<"t"<<t<<endl;
    // }
  
    // std::regex re(R"(\n+)");
    // std::sregex_token_iterator first {filtered_str.begin(),filtered_str.end(),re,-1},last;
    // vector<wstring> wtext_list = {first,last};
    // vector<string> text_list;
    // for(wstring data:wtext_list)
    // {
        
    //     text = to_byte_string(data);  
    //     trim(text);
    //     text_list.push_back(text);
    //     cout<<text<<endl;
    // }
    
    // cout<<text<<endl;
   
    
    return text_list;
}


string _post_replace(string sentence) {
    int i=0;
    for (auto c : sentence)
    {
        if(c=='/')
            sentence.replace(i, 1, "每");
        else if(c=='~')
            sentence.replace(i, 1,"至");
        i++;

    }
    return sentence;
}

// std::wstring _translate(std::wstring sentence) {
//     std::wstring result = sentence;
//     int i=0;
//     for (auto c : sentence) {
//         wstring s(1, c);  
        
//         if (F2H_ASCII_LETTERS.find(s) != F2H_ASCII_LETTERS.end()) {

//             result.replace(i,1,F2H_ASCII_LETTERS.at(s));
//         }
//         else if (F2H_DIGITS.find(s) != F2H_DIGITS.end()) {
//             result.replace(i,1,F2H_DIGITS.at(s));
    
//         } else if (F2H_SPACE.find(s) != F2H_SPACE.end()) {
//             result.replace(i,1,F2H_SPACE.at(s));

//         }
//         i++;
//     }
//     return result;
// }

std::wstring _translate(std::wstring sentence) {
    wstring half;
    for (auto c:sentence) {
        //字符全角转半角
        if (c >= L'Ａ' && c <= L'z')
        {
            half.push_back(c - 65248);
        }
        // //标点全角转半角
        // else if (c >= L'！' && c <= L'～')
        // {
        //     half.push_back(c - 65248);
        // }
        //数字全角转半角
        else if (c >= L'０' && c <= L'９')
        {
            half.push_back(c - 65248);
        }
		else if(c==L'\u3000')	
		{
			
			half.push_back(' ');
		}
        else
        {
            half.push_back(c);
        }
       
    }
    return half;
}


std::string normalize_sentence(std::string sentence) {
    
    wstring wsentence = tranditional_to_simplified(to_wide_string(sentence));
   
    sentence = to_byte_string(_translate(wsentence));    
    sentence = replace(replace_date,sentence,RE_DATE);
    sentence = replace(replace_date2,sentence,RE_DATE2);

    sentence = replace(replace_time,sentence,RE_TIME_RANGE);
    sentence = replace(replace_time,sentence,RE_TIME);
    
    sentence = replace(replace_temperature,sentence,RE_TEMPERATURE);
    sentence = std::regex_replace(sentence,RE_Plus,"$1加$3");
    sentence = std::regex_replace(sentence,RE_Ratio,"$1比$3");

    sentence = replace(replace_frac,sentence,RE_FRAC);
    sentence = replace(replace_percentage,sentence,RE_PERCENTAGE);
    sentence = replace_phonecode_zh(replace_mobile_zh,sentence,RE_MOBILE_PHONE1_zh);
    sentence = replace_phonecode_zh(replace_mobile_zh_,sentence,RE_MOBILE_PHONE2_zh);

    

    sentence = replace_phonecode_zh(replace_phone_zh,sentence,RE_TELEPHONE_zh);
    sentence = replace_phonecode_zh(replace_phone_zh,sentence,RE_NATIONAL_UNIFORM_NUMBER_zh);

    
    sentence = replace(replace_range,sentence,RE_RANGE);
    sentence = replace(replace_number,sentence,RE_DECIMAL_NUM);
    sentence = replace(replace_negative_num,sentence,RE_INTEGER);
    
    
    sentence = replace(replace_positive_quantifier,sentence,RE_POSITIVE_QUANTIFIERS);
    sentence = replace(replace_default_num,sentence,RE_DEFAULT_NUM);
    sentence = replace(replace_number,sentence,RE_NUMBER_);

    sentence = _post_replace(sentence);


    return sentence;
}

// vector<long int> normalize_sentence_(std::string sentence) {
//     vector<long int> time;
//     long int duration_count;
//     auto start = chrono::high_resolution_clock::now();
    
//     wstring wsentence = tranditional_to_simplified(to_wide_string(sentence));
//     auto end = chrono::high_resolution_clock::now();
//     auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "tranditional_to_simplified: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
//     sentence = to_byte_string(_translate(wsentence));

//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "_translate: " << duration.count() << " microseconds" << endl;
//     cout<<sentence<<endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
   
//     sentence = replace(replace_date,sentence,RE_DATE);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_DATE: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
//     sentence = replace(replace_date2,sentence,RE_DATE2);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_DATE2: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
    
//     sentence = replace(replace_time,sentence,RE_TIME_RANGE);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_TIME_RANGE: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
//     sentence = replace(replace_time,sentence,RE_TIME);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_TIME: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();

//     sentence = replace(replace_temperature,sentence,RE_TEMPERATURE);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_TEMPERATURE: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();

//     sentence = replace(replace_frac,sentence,RE_FRAC);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_FRAC: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
//     sentence = replace(replace_percentage,sentence,RE_PERCENTAGE);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_PERCENTAGE: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
//     sentence = replace_phonecode(replace_mobile,sentence,RE_MOBILE_PHONE1);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "MOBILE PHONE : " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();

//     sentence = replace_phonecode(replace_phone,sentence,RE_TELEPHONE);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_TELEPHONE: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
//     sentence = replace_phonecode(replace_phone,sentence,RE_NATIONAL_UNIFORM_NUMBER);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_NATIONAL_UNIFORM_NUMBER: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
    
//     sentence = replace(replace_range,sentence,RE_RANGE);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_RANGE: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
    
//     sentence = replace(replace_negative_num,sentence,RE_INTEGER);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_INTEGER: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
    
//     sentence = replace(replace_number,sentence,RE_DECIMAL_NUM);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_DECIMAL_NUM: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
    
//     sentence = replace(replace_positive_quantifier,sentence,RE_POSITIVE_QUANTIFIERS);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_POSITIVE_QUANTIFIERS: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
    
//     sentence = replace(replace_default_num,sentence,RE_DEFAULT_NUM);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_DEFAULT_NUM: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();
     
//     sentence = replace(replace_number,sentence,RE_NUMBER_);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "RE_NUMBER: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();

//     sentence = _post_replace(sentence);
//     end = chrono::high_resolution_clock::now();
//     duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     duration_count = duration.count();
//     cout << "POST: " << duration.count() << " microseconds" << endl;
//     time.push_back(duration_count);
//     start = chrono::high_resolution_clock::now();



//     return time;
// }


std::vector<std::string> normalize(std::string text) {
       std::vector<std::string> sentences = _split(text);
       std::vector<std::string> normalized_sentences;
       for (std::string sent : sentences) {
           normalized_sentences.push_back(normalize_sentence(sent));
       }
       return normalized_sentences;
}


// int main()
// {
    
//     string text = " +86 13521897850  。 400-666-8800。 010-66122197。   -30° 20° 15℃ -20/30和55/66，5%。-6%。00034,。345，。 00010000。 20~0.5。 20+朵花 。30多岁。 2:15:30。  4:20:10-5:16:10。 2020年3月4日。 2010/03/04。  2250-01-03。Ａ０　聲盤导弹试验是在朝鲜和[],  {}【】  @美国之,间就<=>{}()（）#&@“”^_|…\\平壤核计划的谈判因金正恩与美国前总统唐纳德特朗普之间的峰会失败而陷入停滞之后进行的。  ";
//     // cout<<text<<endl;
//     // vector<string> text_list = _split(text,"zh");
//     //加载繁体-》简体字典
//     Init();
//     vector<string> text_list = normalize(text);
//     for(auto t :text_list)
//     {
        
//         // t = normalize_sentence(t);
//         // cout<<t<<endl;;
//     }
    
//     return 0;
// }