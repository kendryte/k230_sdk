#ifndef TONE_SANHI_H
#define TONE_SANHI_H

#include <iostream>
#include <vector>
#include <unordered_set>
using namespace std;

class ToneSanhi
{
private:
    std::unordered_set<std::string> must_not_neural_tone_words;
    string punc;
    std::unordered_set<std::string> must_neural_tone_words;
    bool _all_tone_three(vector<string> finals);
    bool _is_reduplication(string word);
public:
    ToneSanhi();//构造函数
    std::vector<std::string> _neural_sandhi(std::string word, std::string pos,
                                        std::vector<std::string> finals) ;
    std::vector<std::string> _split_word(std::string word) ;
    vector<string> _yi_sandhi(string word, vector<string> finals);
    vector<string> _three_sandhi(string word, vector<string> finals);
    vector<pair<string, string>> _merge_bu(vector<pair<string, string>> seg);
    vector<pair<string, string>> _merge_yi(vector<pair<string, string>> seg);
    vector<pair<string, string>> _merge_continuous_three_tones(vector<pair<string, string>> seg);
    vector<pair<string, string>> _merge_continuous_three_tones_2(vector<pair<string, string>> seg);
    vector<pair<string, string>> _merge_reduplication(vector<pair<string, string>> seg);
    vector<pair<string, string>> _merge_er(vector<pair<string, string>> seg);
    vector<pair<string, string>> pre_merge_for_modify(vector<pair<string, string>> seg);
    vector<string> modified_tone(string word, string pos,vector<string> finals);
    bool find_string(string word,vector<string> number);
};

#endif