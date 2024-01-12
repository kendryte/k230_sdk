#include <string>  
#include <vector>
#include <regex>

#include "pinyin_utils.h"
#include "char_convert.h"
#include "text_normalization.h"
#include "jieba_utils.h"
#include "zh_frontend.h"



using namespace std;



zh_frontend::zh_frontend()
{
    this->must_erhua = {
        "小院儿", "胡同儿", "范儿", "老汉儿", "撒欢儿", "寻老礼儿", "妥妥儿"
    };
    this->not_erhua = {
        "虐儿", "为儿", "护儿", "瞒儿", "救儿", "替儿", "有儿", "一儿", "我儿", "俺儿", "妻儿",
        "拐儿", "聋儿", "乞儿", "患儿", "幼儿", "孤儿", "婴儿", "婴幼儿", "连体儿", "脑瘫儿",
        "流浪儿", "体弱儿", "混血儿", "蜜雪儿", "舫儿", "祖儿", "美儿", "应采儿", "可儿", "侄儿",
        "孙儿", "侄孙儿", "女儿", "男儿", "红孩儿", "花儿", "虫儿", "马儿", "鸟儿", "猪儿", "猫儿",
        "狗儿"
    };
    this->punc = {"：","，","；","。","？","！","“","”","‘","’","'",":",",",";",".","?","!"};
}

void zh_frontend::_merge_erhua(vector<string>& initials,vector<string>& finals,string word,string pos) {
        if (this->must_erhua.find(word) == this->must_erhua.end() &&
            (this->not_erhua.find(word) != this->not_erhua.end() || pos == "a" ||
             pos == "j" || pos == "nr")) {
            return;
        }
        // "……" 等情况直接返回
        if (finals.size() != word.size()/3) {
            return;
        }
        assert(finals.size() == word.size()/3);
        vector<string> new_initials;
        vector<string> new_finals;
        for (int i = 0; i < finals.size(); i++) {
            if (i == finals.size() - 1 && word.substr(i*3,3) == "儿" &&
                (finals[i] == "er2" || finals[i] == "er5") &&
                this->not_erhua.find(word.substr(word.size() - 2*3)) ==
                    this->not_erhua.end() &&
                !new_finals.empty()) {
                new_finals.back() =
                    new_finals.back().substr(0, new_finals.back().size() - 1) +
                    "r" + new_finals.back().back();
            } else {
                new_finals.push_back(finals[i]);
                new_initials.push_back(initials[i]);
            }
        }
        initials = new_initials;
        finals = new_finals;
        return;
    }

vector<vector<string>> zh_frontend::get_initials_finals(string word) {
    vector<vector<string>> initials_finals;
    vector<string> initials;
    vector<string> finals;
    vector<vector<string>> orig_initials_finals = pypinyin.lazy_pinyin(word, Style::SHENGMU_YUNMU,false,"default",true);
    
    vector<string> orig_initials = orig_initials_finals[0];
    vector<string> orig_finals = orig_initials_finals[1];



    for (int i = 0; i < orig_initials.size(); i++) {
        string c = orig_initials[i];
        string v = orig_finals[i];
        if (regex_match(v, regex("i\\d"))) {
            if (c == "z" || c == "c" || c == "s") {
                v = regex_replace(v, regex("i"), "ii");
            } else if (c == "zh" || c == "ch" || c == "sh" || c == "r") {
                v = regex_replace(v, regex("i"), "iii");
            }
        }
        initials.push_back(c);
        finals.push_back(v);
    }


    initials_finals.push_back(initials);
    initials_finals.push_back(finals);
    return initials_finals;
}
void zh_frontend::splitWord(const string & word, vector<string> & characters)
 {
    int num = word.size();
    int i = 0;
    while(i < num)
     {
         int size = 1;
         if(word[i] & 0x80)
         {
             char temp = word[i];
             temp <<= 1;
             do{
                 temp <<= 1;
                 ++size;
             }while(temp & 0x80);
         }
         string subWord;
         subWord = word.substr(i, size);
         characters.push_back(subWord);
         i += size;
     }
}


vector<vector<string>> zh_frontend::_g2p_fix(vector<string> sentences,
                                               bool merge_sentences,
                                               bool with_erhua) {

    int N=40;
    vector<string> segments = sentences;
    
    vector<vector<string>> phones_list;
    for(auto seg:segments)
    {   
        // # Replace all English words in the sentence
        regex r("[a-zA-Z]+");
        seg = regex_replace(seg, r, "");
        vector<vector<string>> initials;
        vector<vector<string>> finals;
        vector<string> seg_vec;
        splitWord(seg,seg_vec);

        for (auto & word :seg_vec){
            
            vector<vector<string>> sub_initials_finals = get_initials_finals(word);
            
            vector<string> sub_initials = sub_initials_finals[0];
            vector<string> sub_finals = sub_initials_finals[1];
           
            // sub_finals = this->tone_modifier.modified_tone(word,pos,sub_finals);
           
            // if(with_erhua)
            //     _merge_erhua(sub_initials,sub_finals,word,pos);
            
            initials.push_back(sub_initials);
            finals.push_back(sub_finals);
            
        }
        

        vector<vector<string>> initials_list;
        vector<vector<string>> finals_list;
        vector<string> initial;
        for (auto& inner : initials) {
            copy(inner.begin(), inner.end(), back_inserter(initial));
        }
        vector<string> final;
        for (auto& inner : finals) {
            copy(inner.begin(), inner.end(), back_inserter(final));
        }
        if (initial.size()<N)
        {
            initials_list.push_back(initial);
            finals_list.push_back(final);
        }
        else//按照词分割，最长N个字符
        {
            int length = 0;
            vector<string> i_list;
            vector<string> f_list;
            for (int i=0; i<initials.size(); i++) {
                if (i_list.size()+initials[i].size()<N) {
                    i_list.insert(i_list.end(), initials[i].begin(), initials[i].end());
                    f_list.insert(f_list.end(), finals[i].begin(), finals[i].end());
                }
                else {
                    initials_list.push_back(i_list);
                    finals_list.push_back(f_list);
                    i_list.clear();
                    f_list.clear();
                    i_list.insert(i_list.end(), initials[i].begin(), initials[i].end());
                    f_list.insert(f_list.end(), finals[i].begin(), finals[i].end());
                }
            }
            if (i_list.size()>0) {
                initials_list.push_back(i_list);
                finals_list.push_back(f_list);
            }

        }
        vector<string> phones;
        // # NOTE: post process for pypinyin outputs
        // # we discriminate i, ii and iii
        for (int i=0; i<initials_list.size(); i++) {
            for (int j=0; j<initials_list[i].size(); j++) {
                if (initials_list[i][j]!="" && !this->tone_modifier.find_string(initials_list[i][j],this->punc)) {
                    phones.push_back(initials_list[i][j]);
                }
                if (initials_list[i][j]!="" && this->tone_modifier.find_string(initials_list[i][j],this->punc)) {
                    phones.push_back("sp");
                }
                if (finals_list[i][j]!="" &&  !this->tone_modifier.find_string(finals_list[i][j],this->punc)) {
                    phones.push_back(finals_list[i][j]);
                }
            }
            phones_list.push_back(phones);
            phones.clear();
        }  
      
    
    }
    if(merge_sentences)
    {
        vector<string> merge_list;
        for (auto& inner : phones_list) {
            copy(inner.begin(), inner.end(), back_inserter(merge_list));
        }
        if(merge_list.back()=="sp") 
            merge_list.pop_back();
        phones_list.clear();
        phones_list.push_back(merge_list);

    }
    return phones_list;
}

vector<vector<string>> zh_frontend::get_phonemes(string sentence,
                                                   bool merge_sentences,
                                                   bool with_erhua ,
                                                   bool robot,
                                                   bool print_info) {
    cout<<"get_phonemes:"<<sentence<<endl;
    vector<string> sentences = normalize(sentence);



    vector<vector<string>> phonemes = _g2p_fix(sentences, merge_sentences, with_erhua);
    // // change all tones to `1`
    // if (robot) {
    //     vector<vector<string>> new_phonemes;
    //     for (auto sentence : phonemes) {
    //         vector<string> new_sentence;
    //         for (auto item : sentence) {
    //             // `er` only have tone `2`
    //             if (item[item.size() - 1] in "12345" && item != "er2") {
    //                 item = item.substr(0, item.size() - 1) + "1";
    //             }
    //             new_sentence.push_back(item);
    //         }
    //         new_phonemes.push_back(new_sentence);
    //     }
    //     phonemes = new_phonemes;
    // }
    // if (print_info) {
    //     cout << "-----------------------------" << endl;
    //     cout << "text norm results:" << endl;
    //     for (auto sentence : sentences) {
    //         cout << sentence << endl;
    //     }
    //     cout << "-----------------------------" << endl;
    //     cout << "g2p results:" << endl;
    //     for (auto sentence : phonemes) {
    //         for (auto item : sentence) {
    //             cout << item << " ";
    //         }
    //         cout << endl;
    //     }
    //     cout << "-----------------------------" << endl;
    // }
    return phonemes;
}


// int main()
// {
    
//     // string text = "Ａ０ +86 13521897850  。 400-666-8800。 010-66122197。   -30° 20° 15℃ -20/30和55/66，5%。-6%。00034,。345，。 00010000。 20~0.5。 20+朵花 。30多岁。 2:15:30。  4:20:10-5:16:10。 2020年3月4日。 2010/03/04。  2250-01-03。Ａ０　ab聲盤导弹试验是在朝鲜和[],  {}【】  @美国之,间就<=>{}()（）#&@“”^_|…\\平壤核计划的谈判因金正恩与美国前总统唐纳德特朗普之间的峰会失败而陷入停滞之后进行的。  ";
//     string text = "导弹试验是在朝鲜和,美国之间就平壤核计划的谈判因金正恩与美国前总统唐纳德特朗普之间的峰会失败而陷入停滞之后进行的。";
//     cout<<text<<endl;
//     // vector<string> text_list = _split(text,"zh");
//     // vector<pair<string, string> > tagres;
//     // jieba.Tag(text, tagres);
//     // vector<vector<string>> phonemes = get_phonemes(text);
//     // vector<string> text_list = normalize(text);
//     // for(auto t :tagres)
//     // {
//     //     cout<<t<<endl;;
//     // }
    
//     return 0;
// }
    