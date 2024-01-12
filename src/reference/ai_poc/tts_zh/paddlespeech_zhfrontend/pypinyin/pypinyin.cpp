#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>
#include <map>
#include <cstring>
#include <algorithm>
#include "pinyin_utils.h"
#include "_utils.h"
#include "constants.h"
#include "finals.h"
#include "tone.h"
#include "pypinyin.h"
#include <chrono>
#include <unordered_map>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "time.h"
#include "utils_tts.h"


using namespace std;


// int Pypinyin::load_dict(const std::string& path)
// {
//     if (!PINYIN_DICT.empty())
//     {
//         return 0;
//     }
//     std::ifstream infile(path);
//     std::string line;
//     while (std::getline(infile, line))
//     {
//         std::string new_line = line;
//         trim(new_line);
       
//         if (new_line[0] == '#')
//         {
//             continue;
//         }
//         std::vector<std::string> vals, val2;
//         vals = split(new_line, ':');
//         if (vals.size() < 2)
//         {
//             printf("Invalid line:%s\n", new_line.c_str());
//             continue;
//         }
//         std::string utf8_code = vals[0];
        
//         int utf8_code_int;
//         std::stringstream ss;
//         ss << std::hex << utf8_code.substr(2);
//         ss >> utf8_code_int;
//         val2 = split(vals[1], '#');
//         std::string py = val2[0];       
//         PINYIN_DICT[utf8_code_int] = py;
       
//     }
//     return 0;
// }




int Pypinyin::load_dict(const std::string& path)
{
    if (!PINYIN_DICT.empty())
    {
        return 0;
    }
    std::ifstream infile(path);
    std::string line;
    while (std::getline(infile, line))
    {
        std::string new_line = line;
        trim(new_line);
       
        if (new_line[0] == '#')
        {
            continue;
        }
        std::vector<std::string> vals, val2;
        vals = split(new_line, ':');
        if (vals.size() < 2)
        {
            printf("Invalid line:%s\n", new_line.c_str());
            continue;
        }
        std::string utf8_code = vals[0];
        
        int utf8_code_int;
        std::stringstream ss;
        ss << std::hex << utf8_code.substr(2);
        ss >> utf8_code_int;
        val2 = split(vals[1], '#');
        std::string py = val2[0];       
        PINYIN_DICT[utf8_code_int] = py;
       
    }

    
    // size_t totalSize = 0;// 遍历map，计算键和值的内存占用并累加
    // for (const auto& pair : PINYIN_DICT) {
    //     totalSize += sizeof(pair.first);  // 键的内存占用
    //     totalSize += pair.second.size();  // 值的内存占用，这里简单地使用字符串的长度来估算内存占用
    // }

    // // 输出累加后的内存占用
    // std::cout << "Total memory usage of PINYIN_DICT (including keys and values): " << totalSize << " bytes" << std::endl;


    return 0;
}



int Pypinyin::load_phase_dict(const std::string& path)
{
    // int mem = GetSysMemInfo();
    // cout<<"mem left : "<<mem<<endl;
    //记录上一个key，用于判断多音字
    string last_cnstr="";
   
    string key;
    string value;

    std::ifstream infile(path);
    std::string line;
    while (std::getline(infile, line))
    {
        std::string new_line = line;
       
        if (new_line[0] == '#')
        {
            continue;
        }
        std::vector<std::string> vals, val2;
        vals = split(new_line, ':');
        if (vals.size() < 2)
        {
            printf("Invalid line:%s\n", new_line.c_str());
            continue;
        }
        
        key = vals[0];
        trim_shouwei(vals[1]);
        value = vals[1];        

        std::string cnstr = key;
       
        if(cnstr==last_cnstr)//如果存在，说明是多音字
        {
            string last_value = PHRASES_DICT[cnstr];
            if(last_value !=value)
                value = last_value+","+value;
        }
       
        last_cnstr = cnstr;
        PHRASES_DICT[key] = value;

       
    }

    PHRASES_DICT["开户行"]="kai1 hu4 hang2";
    PHRASES_DICT["发卡行"]=  "fa4 ka3 hang2";
    PHRASES_DICT["放款行"]=  "fang4 kuan3 hang2";
    PHRASES_DICT["茧行"]=    "jian3 hang2";
    PHRASES_DICT["行号"]=    "hang2 hao4";
    PHRASES_DICT["各地"]=     "ge4 di4";
    PHRASES_DICT["借还款"]=  "jie4 huan2 kuan3";
    PHRASES_DICT["时间为"]=  "shi2 jian1 wei2";
    PHRASES_DICT["为准"]=    "wei2 zhun3";
    PHRASES_DICT["色差"]=    "se4 cha1"; 
    PHRASES_DICT["嗲"]=      "dia3";
    PHRASES_DICT["呗"]=      "bei5"; 
    PHRASES_DICT["不"]=      "bu4";
    PHRASES_DICT["咗"]=      "zuo5"; 
    PHRASES_DICT["嘞"]=      "lei5";
    PHRASES_DICT["掺和"]=    "chan1 huo5";
    // mem = GetSysMemInfo();
    // cout<<"mem left : "<<mem<<endl;


    // size_t totalSize = 0;// 遍历map，计算键和值的内存占用并累加
    // for (const auto& pair : PHRASES_DICT) {
    //     totalSize += sizeof(pair.first);  // 键的内存占用
    //     totalSize += pair.second.size();  // 值的内存占用，这里简单地使用字符串的长度来估算内存占用
    // }

    // // 输出累加后的内存占用
    // std::cout << "Total memory usage of PHRASES_DICT (including keys and values): " << totalSize << " bytes" << std::endl;



    return 1;
}



// //k230不支持mmap
// int Pypinyin::load_phase_dict(const std::string& path)
// {
//     int mem = GetSysMemInfo();
//     cout<<"mem left : "<<mem<<endl;
//     clock_t start_time,end_time;
//     start_time=clock();
//     int fd = open(path.c_str(), O_RDONLY);
//     if (fd == -1) {
//         std::cerr << "Failed to open file:" <<path<< std::endl;
//         return 0;
//     }
    
//     char* file_data = (char*) mmap(NULL, 8831532, PROT_READ, MAP_PRIVATE, fd, 0);
//     if (file_data == MAP_FAILED) {
//         std::cerr << "Failed to mmap file" << std::endl;
//         return 0;
//     }
    
//     //记录上一个key，用于判断多音字
//     string last_cnstr="";
//     char* end = file_data + 8831532;
//     char* start = file_data;
//     char* line_end;
//     char* key_end;
//     int i=0;
//     string key;
//     string value;
//     while ((line_end = (char *)memchr(start, '\n', end - start)) != NULL) {
//         i++;
        
//         key_end = (char *)memchr(start, ':', line_end - start);
       
//         key.assign(start, key_end - start);
       
//         value.assign(key_end+2, line_end);
       
        
//         start = line_end + 1;
       
//         if (start[0] == '#')
//         {
//             continue;
//         }
      
       
//         std::string cnstr = key;
       
//         if(cnstr==last_cnstr)//如果存在，说明是多音字
//         {
//             string last_value = PHRASES_DICT[cnstr];
//             if(last_value !=value)
//                 value = last_value+","+value;
//         }
       
//         last_cnstr = cnstr;
//         PHRASES_DICT[key] = value;
//         //printf("###%lld  %s %s  %s\n", utf8_code_int,utf8_code.c_str(), py.c_str(), cn.c_str());
         
//     }  
//     munmap(file_data, 8831532);
//     close(fd);
//     end_time=clock();
//     PHRASES_DICT["开户行"]="kai1 hu4 hang2";
//     PHRASES_DICT["发卡行"]=  "fa4 ka3 hang2";
//     PHRASES_DICT["放款行"]=  "fang4 kuan3 hang2";
//     PHRASES_DICT["茧行"]=    "jian3 hang2";
//     PHRASES_DICT["行号"]=    "hang2 hao4";
//     PHRASES_DICT["各地"]=     "ge4 di4";
//     PHRASES_DICT["借还款"]=  "jie4 huan2 kuan3";
//     PHRASES_DICT["时间为"]=  "shi2 jian1 wei2";
//     PHRASES_DICT["为准"]=    "wei2 zhun3";
//     PHRASES_DICT["色差"]=    "se4 cha1"; 
//     PHRASES_DICT["嗲"]=      "dia3";
//     PHRASES_DICT["呗"]=      "bei5"; 
//     PHRASES_DICT["不"]=      "bu4";
//     PHRASES_DICT["咗"]=      "zuo5"; 
//     PHRASES_DICT["嘞"]=      "lei5";
//     PHRASES_DICT["掺和"]=    "chan1 huo5";
    
// 	cout<<"load phase 运行时间"<<(double)(end_time-start_time)/CLOCKS_PER_SEC<<endl;
//     mem = GetSysMemInfo();
//     cout<<"mem left : "<<mem<<endl;
//     return 1;
// }





void Pypinyin::Init(string dict_path,string phase_path){
    
    //加载字典,加载词典
    load_dict(dict_path);
    // cout<<"load_dict success!"<<endl;
    load_phase_dict(phase_path);
    // cout<<"load_phase_dict success!"<<endl;
   
}

std::wregex re_hans(L"^(?:["
                    L"\u3007"                  // 〇
                    L"\u3400-\u4dbf"           // CJK扩展A:[3400-4DBF]
                    L"\u4e00-\u9fff"           // CJK基本:[4E00-9FFF]
                    L"\uf900-\ufaff"           // CJK兼容:[F900-FAFF]
                    L"\U00020000-\U0002A6DF"   // CJK扩展B:[20000-2A6DF]
                    L"\U0002A703-\U0002B73F"   // CJK扩展C:[2A700-2B73F]
                    L"\U0002B740-\U0002B81D"   // CJK扩展D:[2B740-2B81D]
                    L"\U0002F80A-\U0002FA1F"   // CJK兼容扩展:[2F800-2FA1F]
                    L"])+$");

// regex RE_HANS("^(?:["
//         "\\u3007"                  // 〇
//         "\\u3400-\\u4dbf"           // CJK扩展A:[3400-4DBF]
//         "\\u4e00-\\u9fff"           // CJK基本:[4E00-9FFF]
//         "\\uf900-\\ufaff"           // CJK兼容:[F900-FAFF]
//         "\\U00020000-\\U0002A6DF"   // CJK扩展B:[20000-2A6DF]
//         "\\U0002A703-\\U0002B73F"   // CJK扩展C:[2A700-2B73F]
//         "\\U0002B740-\\U0002B81D"   // CJK扩展D:[2B740-2B81D]
//         "\\U0002F80A-\\U0002FA1F"   // CJK兼容扩展:[2F800-2FA1F]
//         "])+$");

// regex RE_HANS_("^(?:["
//         "\\u3007"                  // 〇
//         "\\u3400-\\u4dbf"           // CJK扩展A:[3400-4DBF]
//         "\\u4e00-\\u9fff"           // CJK基本:[4E00-9FFF]
//         "\\uf900-\\ufaff"           // CJK兼容:[F900-FAFF]
//         "])+$");






// 不是pinyin的字符直接返回
std::vector<std::vector<std::string>> Pypinyin::handle_nopinyin(string han, Style style, bool heteronym, string errors, bool strict)
{
    std::vector<std::vector<std::string>> result;
    std::vector<std::string> pys;
    
    pys.push_back(han);
    result.push_back(pys);
    return result;

}

std::vector<std::vector<std::string>> Pypinyin::_single_pinyin(string han, Style style, bool heteronym, string errors, bool strict)
{
    std::vector<std::vector<std::string>> result;
    std::vector<std::string> pys;
    std::vector<int64_t> codes;
    ord(han, codes);

    int64_t num = codes[0];
    auto iter = PINYIN_DICT.find(num);
    if(iter !=PINYIN_DICT.end() )
    {
        
        pys = split(iter->second,',');
    }
    else//处理没有拼音的字符
    {
        return handle_nopinyin(han,style,heteronym,errors,strict);  
    }
    
    result.push_back(pys);
    return result;
}

string Pypinyin::post_convert_style(string pinyin,Style style)
{
    if(style==Style::TONE3 | style==Style::FINALS_TONE3)
    {
        if(!regex_search(pinyin,RE_NUMBER))//没有声调
        {
            return pinyin+"5";
        }
        
    }
    return pinyin;
}

string Pypinyin::convert_style(string pinyin,Style style,bool strict)
{
    string result;
    if(style == Style::FINALS_TONE3)
    {
        result = to_finals_tone3(pinyin,strict);
    }
    else if(style == Style::INITIALS)
    {
        result = get_initials(pinyin, strict);
    }
    else if(style == Style::TONE3)
    {
        result = to_tone3(pinyin);
    }
    result = post_convert_style(result,style);
    return result;
}

/*
获取拼音，并返回声母vector和韵母vector
*/
std::vector<std::vector<std::string>> Pypinyin::_phrase_pinyin(const std::string &phrase, Style style, bool heteronym, const std::string &errors, bool strict)
{
    vector<vector<string>> results;
    vector<int> codes;
    StringArray wphrase = String2StringArray(phrase,codes);
    std::vector<std::vector<std::string>> pinyin_list;
    auto iter = PHRASES_DICT.find(phrase);
   


    if (iter != PHRASES_DICT.end())
    {   
        vector<string> vals = split(iter->second,',');


        for(string val : vals)
        {   
            
            vector<string> v = split(val,' ');

            
            for(auto a:v)
            {   
                pinyin_list.push_back(vector<string>{a});
            }
        }
       
    }
    else
    {
        for(string p:wphrase)
        {
           std::vector<std::vector<std::string>> py = _single_pinyin(p,style,false,"default",true); 
           
           pinyin_list.insert(pinyin_list.end(),py.begin(),py.end());
           
           
        }
    }
    //分别获取声母和韵母

    vector<string> shengmu;
    vector<string> yunmu;
    for (int idx = 0; idx < pinyin_list.size(); idx++) {
        // std::string han = wphrase[idx];
        //多音字直接取第一个值
        
        std::string orig_pinyin = pinyin_list[idx][0];

        if(style!=Style::SHENGMU_YUNMU)
        {
            pinyin_list[idx] = {
                convert_style(orig_pinyin, style,strict)};
            
        }
        else
        {
            vector<string> initials_finals;
            initials_finals =  to_initials_finals_tone3(orig_pinyin,strict);



            if(initials_finals.size()<2)
                initials_finals.push_back(initials_finals[0]);

            initials_finals[1] = post_convert_style(initials_finals[1],Style::FINALS_TONE3);
            shengmu.push_back(initials_finals[0]);
            yunmu.push_back(initials_finals[1]);
        }
        
        
    }
    if(style!=Style::SHENGMU_YUNMU)
        return pinyin_list;
    else{
        results.push_back(shengmu);
        results.push_back(yunmu);
        return results;
    }    
}


vector<vector<string>> Pypinyin::lazy_pinyin(const string &words, Style style, bool heteronym, const string &errors, bool strict) {
    vector<vector<string>> pys;
    // 初步过滤没有拼音的字符


    wstring wwords = to_wide_string(words);
    if (std::regex_match(wwords, re_hans))
    {
        pys = _phrase_pinyin(words, style, heteronym, errors, strict);
        
    
        if(style == Style::SHENGMU_YUNMU)
            return pys;
        else
        {
            std::vector<std::string> result;
            for (auto py : pys) {
                result.insert(result.end(), py.begin(), py.end());
            }
            return {result};
        }
        
    }
    pys = handle_nopinyin(words, style, heteronym,errors, strict);
    if(style == Style::SHENGMU_YUNMU)
    {
        if(pys.size()<2)
        {
            pys.push_back(pys[0]);
        }
        return pys;
    }
    else
    {
        std::vector<std::string> result;
        for (auto py : pys) {
            result.insert(result.end(), py.begin(), py.end());
        }

        return {result};
    }

}


// int main()
// {
//     string dict_path = "/data/zhoumeng/TTS/TensorFlowTTS/examples/cpptflite/paddlespeech_zhfrontend/pypinyin/pinyin.txt";
//     string phase_path = "/data/zhoumeng/TTS/TensorFlowTTS/examples/cpptflite/paddlespeech_zhfrontend/pypinyin/large_pinyin.txt";
//     Pypinyin pypinyin;
//     pypinyin.Init(dict_path,phase_path);
//     while (std::cin)
//    {
//         std::string s;
//         std::getline(std::cin, s);
//         // vector<int> size;
//         // StringArray a = String2StringArray(s,size);
//         // for(string pp:a)
//         //     cout<<pp<<endl;
//         // std::vector<std::vector<std::string>> result = _single_pinyin("好",Style::TONE3,false,"default",true);
        
//         vector<std::vector<std::string>> result = pypinyin.lazy_pinyin(s,Style::FINALS_TONE3,false,"default",true);
//         // vector<std::vector<std::string>> result = _phrase_pinyin(s,Style::INITIALS,false,"default",true);
//         // vector<std::vector<std::string>> result1 = _phrase_pinyin(s,Style::FINALS_TONE3,false,"default",true);
//         // vector<std::vector<std::string>> result2 = _phrase_pinyin(s,Style::TONE3,false,"default",true);
//         // vector<std::vector<std::string>> result3 = _phrase_pinyin(s,Style::SHENGMU_YUNMU,false,"default",true);
//         // std::vector<std::vector<std::string>> result = lazy_pinyin(s,Style::FINALS_TONE3,false,"default",true)
//         for(auto p:result)
//             for(auto pp:p)
//                 cout<<pp<<endl;
//         result = pypinyin.lazy_pinyin(s,Style::SHENGMU_YUNMU,false,"default",true);
//         for(auto p:result)
//             for(auto pp:p)
//                 cout<<pp<<endl;
//    }
//     return 0;
    
// }

