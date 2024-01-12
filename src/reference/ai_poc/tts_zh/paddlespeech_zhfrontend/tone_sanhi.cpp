#include <iostream>
#include <unordered_set>

#include "tone_sanhi.h"
#include "jieba_utils.h"
 
using namespace std;


ToneSanhi::ToneSanhi() {
    this->must_not_neural_tone_words = {
    "男子", "女子", "分子", "原子", "量子", "莲子", "石子", "瓜子", "电子", "人人", "虎虎"
    };
    this->punc = "：，；。？！“”‘’':,;.?!";
    this->must_neural_tone_words = {
    "麻烦", "麻利", "鸳鸯", "高粱", "骨头", "骆驼", "马虎", "首饰", "馒头", "馄饨", "风筝",
    "难为", "队伍", "阔气", "闺女", "门道", "锄头", "铺盖", "铃铛", "铁匠", "钥匙", "里脊",
    "里头", "部分", "那么", "道士", "造化", "迷糊", "连累", "这么", "这个", "运气", "过去",
    "软和", "转悠", "踏实", "跳蚤", "跟头", "趔趄", "财主", "豆腐", "讲究", "记性", "记号",
    "认识", "规矩", "见识", "裁缝", "补丁", "衣裳", "衣服", "衙门", "街坊", "行李", "行当",
    "蛤蟆", "蘑菇", "薄荷", "葫芦", "葡萄", "萝卜", "荸荠", "苗条", "苗头", "苍蝇", "芝麻",
    "舒服", "舒坦", "舌头", "自在", "膏药", "脾气", "脑袋", "脊梁", "能耐", "胳膊", "胭脂",
    "胡萝", "胡琴", "胡同", "聪明", "耽误", "耽搁", "耷拉", "耳朵", "老爷", "老实", "老婆",
    "老头", "老太", "翻腾", "罗嗦", "罐头", "编辑", "结实", "红火", "累赘", "糨糊", "糊涂",
    "精神", "粮食", "簸箕", "篱笆", "算计", "算盘", "答应", "笤帚", "笑语", "笑话", "窟窿",
    "窝囊", "窗户", "稳当", "稀罕", "称呼", "秧歌", "秀气", "秀才", "福气", "祖宗", "砚台",
    "码头", "石榴", "石头", "石匠", "知识", "眼睛", "眯缝", "眨巴", "眉毛", "相声", "盘算",
    "白净", "痢疾", "痛快", "疟疾", "疙瘩", "疏忽", "畜生", "生意", "甘蔗", "琵琶", "琢磨",
    "琉璃", "玻璃", "玫瑰", "玄乎", "狐狸", "状元", "特务", "牲口", "牙碜", "牌楼", "爽快",
    "爱人", "热闹", "烧饼", "烟筒", "烂糊", "点心", "炊帚", "灯笼", "火候", "漂亮", "滑溜",
    "溜达", "温和", "清楚", "消息", "浪头", "活泼", "比方", "正经", "欺负", "模糊", "槟榔",
    "棺材", "棒槌", "棉花", "核桃", "栅栏", "柴火", "架势", "枕头", "枇杷", "机灵", "本事",
    "木头", "木匠", "朋友", "月饼", "月亮", "暖和", "明白", "时候", "新鲜", "故事", "收拾",
    "收成", "提防", "挖苦", "挑剔", "指甲", "指头", "拾掇", "拳头", "拨弄", "招牌", "招呼",
    "抬举", "护士", "折腾", "扫帚", "打量", "打算", "打点", "打扮", "打听", "打发", "扎实",
    "扁担", "戒指", "懒得", "意识", "意思", "情形", "悟性", "怪物", "思量", "怎么", "念头",
    "念叨", "快活", "忙活", "志气", "心思", "得罪", "张罗", "弟兄", "开通", "应酬", "庄稼",
    "干事", "帮手", "帐篷", "希罕", "师父", "师傅", "巴结", "巴掌", "差事", "工夫", "岁数",
    "屁股", "尾巴", "少爷", "小气", "小伙", "将就", "对头", "对付", "寡妇", "家伙", "客气",
    "实在", "官司", "学问", "学生", "字号", "嫁妆", "媳妇", "媒人", "婆家", "娘家", "委屈",
    "姑娘", "姐夫", "妯娌", "妥当", "妖精", "奴才", "女婿", "头发", "太阳", "大爷", "大方",
    "大意", "大夫", "多少", "多么", "外甥", "壮实", "地道", "地方", "在乎", "困难", "嘴巴",
    "嘱咐", "嘟囔", "嘀咕", "喜欢", "喇嘛", "喇叭", "商量", "唾沫", "哑巴", "哈欠", "哆嗦",
    "咳嗽", "和尚", "告诉", "告示", "含糊", "吓唬", "后头", "名字", "名堂", "合同", "吆喝",
    "叫唤", "口袋", "厚道", "厉害", "千斤", "包袱", "包涵", "匀称", "勤快", "动静", "动弹",
    "功夫", "力气", "前头", "刺猬", "刺激", "别扭", "利落", "利索", "利害", "分析", "出息",
    "凑合", "凉快", "冷战", "冤枉", "冒失", "养活", "关系", "先生", "兄弟", "便宜", "使唤",
    "佩服", "作坊", "体面", "位置", "似的", "伙计", "休息", "什么", "人家", "亲戚", "亲家",
    "交情", "云彩", "事情", "买卖", "主意", "丫头", "丧气", "两口", "东西", "东家", "世故",
    "不由", "不在", "下水", "下巴", "上头", "上司", "丈夫", "丈人", "一辈", "那个", "菩萨",
    "父亲", "母亲", "咕噜", "邋遢", "费用", "冤家", "甜头", "介绍", "荒唐", "大人", "泥鳅",
    "幸福", "熟悉", "计划", "扑腾", "蜡烛", "姥爷", "照顾", "喉咙", "吉他", "弄堂", "蚂蚱",
    "凤凰", "拖沓", "寒碜", "糟蹋", "倒腾", "报复", "逻辑", "盘缠", "喽啰", "牢骚", "咖喱",
    "扫把", "惦记"
    };
};

bool ToneSanhi::find_string(string word,vector<string> number)
{  
    for(auto n:number)
    {
        std::size_t pos = word.find(n);
        if (pos != std::string::npos) 
            return true;      
    }
    return false;

}

vector<string> ToneSanhi::_split_word(string word) {
   
    vector<string> word_list;
    // jieba.CutForSearch(word, word_list);
    
    sort(word_list.begin(), word_list.end(), [](string a, string b) {
        return a.size() < b.size();
    });
    
    string first_subword = word_list[0];
    int first_begin_idx = word.find(first_subword);
    vector<string> new_word_list;
    if (first_begin_idx == 0) {
        string second_subword = word.substr(first_subword.size());
        
        new_word_list = {first_subword, second_subword};
    } else {
        string second_subword = word.substr(0, word.size() - first_subword.size());
        new_word_list = {second_subword, first_subword};
    }
    return new_word_list;
}

vector<string> ToneSanhi::_neural_sandhi(string word, string pos,
                                        vector<string> finals) 
{
  // reduplication words for n. and v. e.g. 奶奶, 试试, 旺旺
  //中文utf-8编码占3个字节
    for (int j = 0; j < word.size(); j+=3) 
    {
      if (j - 3 >= 0 && word.substr(j,3) == word.substr(j - 3,3) && (pos[0] == 'n' || pos[0] == 'v' ||
          pos[0] == 'a') &&
          this->must_not_neural_tone_words.find(word) == this->must_not_neural_tone_words.end()) 
        {
            if(word=="空空")
                continue;
            finals[j/3] = finals[j/3].substr(0, finals[j/3].size() - 1) + "5";
        }
    }

    int ge_idx = word.find("个");
    if(word.size() >= 3 && find_string(word.substr(word.size() - 3),vector<string>{"吧","呢","哈","啊","呐","噻","嘛","吖","嗨","呐","哦","哒","额","滴","哩","哟","喽","啰","耶","喔","诶"}))
    {
        finals[word.size()/3 - 1] = finals[word.size()/3 - 1].substr(0, finals[word.size()/3 - 1].size() - 1) + "5";
    }
    else if (word.size() >= 3 && find_string(word.substr(word.size() - 3) ,{"的","地","得"}))
    {
        finals[word.size()/3 - 1] = finals[word.size()/3 - 1].substr(0, finals[word.size()/3 - 1].size() - 1) + "5";
    } else if ((word.size()/3 == 1) && find_string(word,{"了","着","过",})&& (pos == "ul" || pos == "uz" || pos == "ug"))
    {
        finals[word.size()/3 - 1] = finals[word.size()/3 - 1].substr(0, finals[word.size()/3 - 1].size() - 1) + "5";
    } else if (word.size() > 3 && find_string(word.substr(word.size()-3),{"们","子"}) &&(pos == "r" || pos == "n") &&
               this->must_not_neural_tone_words.find(word) == this->must_not_neural_tone_words.end()) {
        finals[word.size()/3 - 1] = finals[word.size()/3 - 1].substr(0, finals[word.size()/3 - 1].size() - 1) + "5";
    } else if (word.size() > 3 && find_string(word.substr(word.size() - 3),{"上","下","里"}) &&(pos == "s" || pos == "l" || pos == "f"))
    {
        finals[word.size()/3 - 1] = finals[word.size()/3 - 1].substr(0, finals[word.size()/3 - 1].size() - 1) + "5";
    } else if (word.size() > 3 && find_string(word.substr(word.size() - 1*3,3),{"来","去"}) && find_string(word.substr(word.size() - 2*3,3),{"上","下","进","出","回","过","起","开"})) {
        finals[word.size()/3 - 1] = finals[word.size()/3 - 1].substr(0, finals[word.size()/3 - 1].size() - 1) + "5";
    } 
    else if (ge_idx >= 3 &&
                (find_string(word.substr(ge_idx-3 ,3),{"0","1","2","3","4","5","6","7","8","9","零","一","二","三","四","五","六","七","八","九","十","几","两","多","整","做","有","半","各","每","是"})) ||
                word == "个") {
        finals[ge_idx/3] = finals[ge_idx/3].substr(0, finals[ge_idx/3].size() - 1) + "5";
    } 
    else {
        if(word.size()>=6)
            if (this->must_neural_tone_words.find(word) != this->must_neural_tone_words.end() ||
            this->must_neural_tone_words.find(word.substr(word.size() - 2*3, 2*3)) != this->must_neural_tone_words.end()) {
               
               finals[finals.size() - 1] = finals[finals.size() - 1].substr(0, finals[finals.size() - 1].size() - 1) + "5";
            }
    }
    
    vector<string> word_list = _split_word(word);
    vector<vector<string>> finals_list;

    finals_list.push_back(vector<string>(finals.begin(), finals.begin() + word_list[0].size()/3));
    
    finals_list.push_back(vector<string>(finals.begin() + word_list[0].size()/3, finals.end()));
    
    for (int i = 0; i < word_list.size(); i++) {
      // conventional neural in Chinese
        if (this->must_neural_tone_words.find(word_list[i]) != this->must_neural_tone_words.end() ||(word_list[i].size()>=6 &&
            this->must_neural_tone_words.find(word_list[i].substr(word_list[i].size() - 6, 6)) != this->must_neural_tone_words.end())) {
            finals_list[i][word_list[i].size()/3 - 1] = finals_list[i][word_list[i].size()/3 - 1].substr(0, finals_list[i][word_list[i].size()/3 - 1].size() - 1) + "5";
        }
    }
   
    finals = finals_list[0];
    finals.insert(finals.end(), finals_list[1].begin(), finals_list[1].end());
    return finals;
}

vector<string> _bu_sandhi(string word,vector<string> finals) {
    // e.g. 看不懂
    if (word.length() == 9 && word.substr(3,3) == "不") {
        finals[1] = finals[1].substr(0,finals[1].length()-1) + "5";
    }
    else {
        for (int i = 0; i < word.length(); i+=3) {
            // "不" before tone4 should be bu2, e.g. 不怕
            if ((word.substr(i,3) == "不") && ((i + 3) < word.length()) && (finals[i/3 + 1].back() == '4')) {
                finals[i/3] = finals[i/3].substr(0,finals[i/3].length()-1) + "2";
            }
        }
    }
    return finals;
}



vector<string> ToneSanhi::_yi_sandhi(string word, vector<string> finals) {
    // "一" in number sequences, e.g. 一零零, 二一零   
    vector<string> number{"零","二","三","四","五","六","七","八","九","十"};
    if (word.find("一") != string::npos && find_string(word,number)) {
        return finals;
    }
    // "一" between reduplication words shold be yi5, e.g. 看一看
    else if (word.length() == 9 && word.substr(3,3) == "一" && word.substr(0,3) == word.substr(word.length()-3,3)) {
        finals[1] = finals[1].substr(0, finals[1].length() - 1) + "5";
    }
    // when "一" is ordinal word, it should be yi1
    else if (word.substr(0,6)=="第一") {
        finals[1] = finals[1].substr(0, finals[1].length() - 1) + "1";
    }
    else {
        for (int i = 0; i < word.length(); i+=3) {
            if (word.substr(i,3) == "一" && i + 3 < word.length()) {
                // "一" before tone4 should be yi2, e.g. 一段
                if((word == "一日")||(word == "一班")||(word == "一月"))
                {
                    return finals;
                }
                else if (finals[i/3 + 1][finals[i/3 + 1].length()-1] == '4') {
                    finals[i/3] = finals[i/3].substr(0, finals[i/3].length() - 1) + "2";
                }
                // "一" before non-tone4 should be yi4, e.g. 一天
                else {
                    // "一" 后面如果是标点，还读一声
                    if (!find_string(word.substr(i+1,3),{"：","，","；","。","？","！","“","”","‘","’","'",":",",",";",".","?","!"})) {
                        finals[i/3] = finals[i/3].substr(0, finals[i/3].length() - 1) + "4";
                    }
                }
            }
        }
    }
    return finals;
}


vector<string> ToneSanhi::_three_sandhi(string word, vector<string> finals) {
    if (word.size() == 6 && _all_tone_three(finals)) {
        finals[0] = finals[0].substr(0, finals[0].size() - 1) + "2";
    } 
    else if (word.size() == 9) {
        vector<string> word_list = _split_word(word);
        if (_all_tone_three(finals)) {
            //  disyllabic + monosyllabic, e.g. 蒙古/包
            if (word_list[0].size() == 6) {
                finals[0] = finals[0].substr(0, finals[0].size() - 1) + "2";
                finals[1] = finals[1].substr(0, finals[1].size() - 1) + "2";
            }
            //  monosyllabic + disyllabic, e.g. 纸/老虎
            else if (word_list[0].size() == 3) {
                finals[1] = finals[1].substr(0, finals[1].size() - 1) + "2";
            }
        } else {
            vector<vector<string>> finals_list = {
                    vector<string>(finals.begin(), finals.begin() + word_list[0].size()/3),
                    vector<string>(finals.begin() + word_list[0].size()/3, finals.end())
            };
            if (finals_list.size() == 2) {
                for (int i = 0; i < finals_list.size(); i++) {
                    // e.g. 所有/人
                    if (_all_tone_three(finals_list[i]) && finals_list[i].size() == 2) {
                        finals_list[i][0] = finals_list[i][0].substr(0, finals_list[i][0].size() - 1) + "2";
                    }
                    // e.g. 好/喜欢
                    else if (i == 1 && !_all_tone_three(finals_list[i]) && finals_list[i][0][finals_list[i][0].size() - 1] == '3' &&
                             finals_list[0][finals_list[0].size() - 1][finals_list[0][finals_list[0].size() - 1].size() - 1] == '3')
                    {
                        finals_list[0][finals_list[0].size() - 1] = finals_list[0][finals_list[0].size() - 1].substr(0,
                                                                                                                        finals_list[0][
                                                                                                                                finals_list[0].size() - 1].size() - 1) + "2";
                    }
                }
                    finals = finals_list[0];
                    finals.insert(finals.end(), finals_list[1].begin(), finals_list[1].end());
            }
        }
    }
    // split idiom into two words who's length is 2
    else if (word.size() == 12) {
        vector<vector<string>> finals_list = {
                vector<string>(finals.begin(), finals.begin() + 2),
                vector<string>(finals.begin() + 2, finals.end())
        };
        finals = {};
        for (auto sub : finals_list) {
            if (_all_tone_three(sub)) {
                sub[0] = sub[0].substr(0, sub[0].size() - 1) + "2";
            }
            finals.insert(finals.end(), sub.begin(), sub.end());
        }
    }
    return finals;
}


/*
// merge "不" and the word behind it
// if don't merge, "不" sometimes appears alone according to jieba, which may occur sandhi error
*/
vector<pair<string, string>> ToneSanhi::_merge_bu(vector<pair<string, string>> seg) {
    vector<pair<string, string>> new_seg;
    string last_word = "";
    for (auto word_pos : seg) {
        if (last_word == "不") {
            word_pos.first = last_word + word_pos.first;
        }
        if (word_pos.first != "不") {
            new_seg.push_back(word_pos);
        }
        last_word = word_pos.first;
    }
    if (last_word == "不") {
        new_seg.push_back(make_pair(last_word, "d"));
        last_word = "";
    }
    return new_seg;
}

/*
// function 1: merge "一" and reduplication words in it's left and right, e.g. "听","一","听" ->"听一听"
// function 2: merge single  "一" and the word behind it
// if don't merge, "一" sometimes appears alone according to jieba, which may occur sandhi error
// e.g.
// input seg: [('听', 'v'), ('一', 'm'), ('听', 'v')]
// output seg: [['听一听', 'v']]
*/
vector<pair<string, string>> ToneSanhi::_merge_yi(vector<pair<string, string>> seg) {
    vector<pair<string, string>> new_seg;
    // function 1
    for (int i = 0; i < seg.size(); i++) {
        if (i - 1 >= 0 && seg[i].first == "一" && i + 1 < seg.size() && seg[i - 1].first == seg[i + 1].first && seg[i - 1].second == "v") {
            new_seg[i - 1].first = new_seg[i - 1].first + "一" + new_seg[i - 1].first;
        } else {
            if (i - 2 >= 0 && seg[i - 1].first == "一" && seg[i - 2].first == seg[i].first && seg[i].second == "v") {
                continue;
            } else {
                new_seg.push_back(seg[i]);
            }
        }
    }
    seg = new_seg;
    new_seg.clear();
    // function 2
    for (int i = 0; i < seg.size(); i++) {
        if (!new_seg.empty() && new_seg[new_seg.size() - 1].first == "一") {
            new_seg[new_seg.size() - 1].first = new_seg[new_seg.size() - 1].first + seg[i].first;
        } else {
            new_seg.push_back(seg[i]);
        }
    }
    return new_seg;
}


bool ToneSanhi::_is_reduplication(string word) {
    if((word.size() == 6)&&(word.substr(0,3)== word.substr(3,6)))
        return true;
    else
        return false;
}

bool ToneSanhi::_all_tone_three(vector<string> finals) {
    for (auto &x : finals) {
        if (x.back() != '3') {
            return false;
        }
    }
    return true;
}

vector<pair<string, string>> ToneSanhi::_merge_continuous_three_tones(vector<pair<string, string>> seg) 
{
    vector<pair<string, string>> new_seg;
    
    vector<vector<string>> sub_finals_list;
    
    for (const auto &[word, pos] : seg) {
        
        sub_finals_list.emplace_back(
            pypinyin.lazy_pinyin(word,Style::FINALS_TONE3,false,"default",true)[0]);    
    }
    assert(sub_finals_list.size() == seg.size());
    vector<bool> merge_last(seg.size(), false);
    for (int i = 0; i < seg.size(); ++i) {
        const auto &[word, pos] = seg[i];
        if ((i - 1) >= 0)
            if(_all_tone_three(sub_finals_list[i - 1]) &&_all_tone_three(sub_finals_list[i]) && !merge_last[i - 1]) {
            // if the last word is reduplication, not merge, because reduplication need to be _neural_sandhi
                if (!_is_reduplication(seg[i - 1].first) &&
                    seg[i - 1].first.size() + seg[i].first.size() <= 9) {
                    new_seg.back().first += seg[i].first;
                    merge_last[i] = true;
                } else {
                    new_seg.emplace_back(word, pos);
                }
            }
            else {
                new_seg.emplace_back(word, pos);
            }
        else
        {           
            new_seg.emplace_back(word, pos);           
        }
        
    }
    return new_seg;
}

// # the last char of first word and the first char of second word is tone_three
vector<pair<string, string>> ToneSanhi::_merge_continuous_three_tones_2(vector<pair<string, string>> seg) 
{
    vector<pair<string, string>> new_seg;
    
    vector<vector<string>> sub_finals_list;
    
    for (const auto &[word, pos] : seg) {
        
        sub_finals_list.emplace_back(
            pypinyin.lazy_pinyin(word,Style::FINALS_TONE3,false,"default",true)[0]);    
    }
    assert(sub_finals_list.size() == seg.size());
    vector<bool> merge_last(seg.size(), false);
    for (int i = 0; i < seg.size(); ++i) {
        const auto &[word, pos] = seg[i];
        if ((i - 1) >= 0)
            if((sub_finals_list[i - 1][sub_finals_list[i - 1].size()-1].back()=='3') &&(sub_finals_list[i][0].back()=='3') && !merge_last[i - 1]) {
            // if the last word is reduplication, not merge, because reduplication need to be _neural_sandhi
                if (!_is_reduplication(seg[i - 1].first) &&
                    seg[i - 1].first.size() + seg[i].first.size() <= 9) {
                    new_seg.back().first += seg[i].first;
                    merge_last[i] = true;
                } else {
                    new_seg.emplace_back(word, pos);
                }
            }
            else {
                new_seg.emplace_back(word, pos);
            }
        else
        {           
            new_seg.emplace_back(word, pos);           
        }
        
    }
    return new_seg;
}


vector<pair<string, string>> ToneSanhi::_merge_reduplication(vector<pair<string, string>> seg) {   
    vector<pair<string, string>> new_seg;
    for (int i = 0; i < seg.size(); i++) {
        if (new_seg.size() && seg[i].first == new_seg.back().first) {
            new_seg.back().first += seg[i].first;
        } else {
            new_seg.push_back(seg[i]);
        }
    }
    return new_seg;
}

vector<pair<string, string>> ToneSanhi::_merge_er(vector<pair<string, string>> seg) {
    vector<pair<string, string>> new_seg;
    for (int i = 0; i < seg.size(); i++) {
        if (i - 1 >= 0 && seg[i].first == "儿") {
            new_seg[new_seg.size() - 1].first += seg[i].first;
        } else {
            new_seg.push_back(seg[i]);
        }
    }
    return new_seg;
}

vector<pair<string, string>> ToneSanhi::pre_merge_for_modify(vector<pair<string, string>> seg) {
    seg = _merge_bu(seg);
    seg = _merge_yi(seg);
    seg = _merge_reduplication(seg);
    seg = _merge_continuous_three_tones(seg);
    seg = _merge_continuous_three_tones_2(seg);
    seg = _merge_er(seg);
    return seg;
}

vector<string> ToneSanhi::modified_tone(string word, string pos,vector<string> finals) {
   
    finals = _bu_sandhi(word, finals);
    finals = _yi_sandhi(word, finals);
    finals = _neural_sandhi(word, pos, finals);
    finals = _three_sandhi(word, finals);
    
    return finals;
}


// int main()
// {
    
//     // string text = "Ａ０ +86 13521897850  。 400-666-8800。 010-66122197。   -30° 20° 15℃ -20/30和55/66，5%。-6%。00034,。345，。 00010000。 20~0.5。 20+朵花 。30多岁。 2:15:30。  4:20:10-5:16:10。 2020年3月4日。 2010/03/04。  2250-01-03。Ａ０　ab聲盤导弹试验是在朝鲜和[],  {}【】  @美国之,间就<=>{}()（）#&@“”^_|…\\平壤核计划的谈判因金正恩与美国前总统唐纳德特朗普之间的峰会失败而陷入停滞之后进行的。  ";
//     string text = "纸老虎 蒙古包 好喜欢 所有人好不好不知道不道听一听听听喔喔我我我我儿";
    
    
    
//     vector<pair<string, string> > tagres;
//     jieba.Tag(text, tagres);
    
//     ToneSanhi ToneSanhi;
//     // vector<string> text_list = ToneSanhi._split_word(text);
//     vector<pair<string, string>> text_list = ToneSanhi.pre_merge_for_modify(tagres);
    
//     // vector<string> text_list = _split(text,"zh");
//     // vector<pair<string, string> > tagres;
//     // jieba.Tag(text, tagres);
//     // vector<vector<string>> phonemes = get_phonemes(text);
//     // vector<string> text_list = normalize(text);
//     string result = "";
//     for(auto t :text_list)
//     {
//         result += (t.first+t.second);
//     }
//     cout<<result<<endl;
    
//     return 0;
// }


