#ifndef PYPINYIN_H
#define PYPINYIN_H
#include <iostream>
#include <unordered_map>
#include "constants.h"

using namespace std;
class Pypinyin {
    public:
        // Pypinyin(string dict_path, string phase_path) {
        // Init(dict_path, phase_path);
        // }
        void Init(string dict_path,string phase_path);
        vector<vector<string>> lazy_pinyin(const string &words, Style style, bool heteronym, const string &errors, bool strict);
        std::unordered_map <int, std::string> PINYIN_DICT;
        std::map <std::string, std::string> PHRASES_DICT;
        

    private:
        
        int load_dict(const std::string& path);
        int load_phase_dict(const std::string& path);
        std::vector<std::vector<std::string>> handle_nopinyin(string han, Style style, bool heteronym, string errors, bool strict);
        std::vector<std::vector<std::string>> _single_pinyin(string han, Style style, bool heteronym, string errors, bool strict);
        string post_convert_style(string pinyin,Style style);
        string convert_style(string pinyin,Style style,bool strict);
        std::vector<std::vector<std::string>> _phrase_pinyin(const std::string &phrase, Style style, bool heteronym, const std::string &errors, bool strict);

};




#endif