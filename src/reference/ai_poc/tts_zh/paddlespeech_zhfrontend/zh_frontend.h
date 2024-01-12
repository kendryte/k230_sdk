#ifndef ZH_FRONTEND_H
#define ZH_FRONTEND_H


#include <iostream>
#include <vector>
#include "tone_sanhi.h"
using namespace std;

class zh_frontend
{
private:
    ToneSanhi tone_modifier;
    vector<vector<string>> get_initials_finals(string word);
    unordered_set<string> must_erhua;
    unordered_set<string> not_erhua;
    vector<string> punc;
    void _merge_erhua(vector<string>& initials,vector<string>& finals,string word,string pos);
    
public:
    zh_frontend();//构造函数
    void splitWord(const string & word, vector<string> & characters);
    vector<vector<string>> _g2p_fix(vector<string> sentences,
                                               bool merge_sentences,
                                               bool with_erhua);
    vector<vector<string>> get_phonemes(string sentence,
                                                   bool merge_sentences,//merge_sentences = true
                                                   bool with_erhua ,//with_erhua = true
                                                   bool robot ,//robot = false
                                                   bool print_info);//print_info = false
    
    
};




#endif