/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <thread>
#include <cmath>
#include <cctype>
#include "VoxCommon.h"
#include "fastspeech1.h"
#include "fastspeech2.h"
#include "hifigan.h"
#include "zh_frontend.h"
#include "jieba_utils.h"
#include "length_regulator.h"
using std::cerr;
using std::cout;
using std::endl;

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_fastspeech1> <kmodel_fastspeech2> <kmodel_hifigan> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_fastspeech1      中文tts encoder kmodel路径\n"
         << "  kmodel_fastspeech2      中文tts decoder kmodel路径\n"
         << "  kmodel_hifigan          中文tts 声码器  kmodel路径\n"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

int _symbols_to_sequence(string s,map<string,int> symbol_to_id)
{
    int id;
    
    if((symbol_to_id.find(s) != symbol_to_id.end())&&(s != "_")&&(s != "~"))
        id = symbol_to_id[s];
        
    else if(s.find_first_of("：，；。？！“”‘’':,;.?!") != string::npos)
        id = symbol_to_id["sp"];
    else
        id = symbol_to_id["sp"];
    return id;

}

int main(int argc, char *argv[])
{  
    int N=50;
    int M = 100;
    int C = 80;
    int Dim = 256;
    int L = 600;
    int M_pad = 0;
    int times = 0;
    map<string, int> symbol_to_id;
    map<string, int> vocab_phones_mix;
    //创建zh_frontend对象，实现拼音转音素的过程
    zh_frontend zh_frontend;
    //加载拼音字典
    string dict_path = "file/pinyin_txt/pinyin.txt";
    string phase_path = "file/pinyin_txt/small_pinyin.txt";
    pypinyin.Init(dict_path, phase_path);
    {
        std::ifstream file_en("file/phone_id_map_en.txt");
        std::string line;
        while (std::getline(file_en, line)) {
            std::istringstream iss(line);
            std::string str;
            int num;
            iss >> str >> num;
            symbol_to_id[str] = num;
        }
        file_en.close();
    }
    //初始化模型
    Fastspeech1 fspeech1(argv[1],atoi(argv[4]));
    Fastspeech2 fspeech2(argv[2],atoi(argv[4]));
    HiFiGan hifigan(argv[3],atoi(argv[4]));

    while(times < 3)
    {   
        vector<float> results_mel_all;
        vector<vector<int>> sequence_list;
        std::cout << "请输入需要文本内容：";
        std::string text;
        getline(std::cin, text); // 获取用户输入的一行文本
        //解析得到的音素数据
        vector<string> result_phonemes;
        //每个拆分的子音素序列中有效数据的长度列表
        vector<int> padding_phonemes;
        //音素序列
        vector<int> sequence;

        regex RE_DOUHAO_NUM("([0-9])([\\,])([0-9])");        
        text = std::regex_replace(text,RE_DOUHAO_NUM,"$1$3"); 
        //文本转拼音
        vector<vector<string>> pinyin = zh_frontend.get_phonemes(text,false,true,false,false);
        //拼音转音素
        for (vector<std::string>& t : pinyin) {
            if (t[t.size() - 1] == "\n") {
                t.pop_back(); 
            }
            result_phonemes.insert(result_phonemes.end(), t.begin(), t.end());
        }
        //result_phonemes是预处理得到的音素列表，因为fastspeech1模型的输入是定长(1,50)
        //因此，如果一个句子的音素超过50需要拆分成多个50处理
        for(string t : result_phonemes) {
            if (sequence.size()<N)
                sequence.push_back(_symbols_to_sequence(t,symbol_to_id));
            else
            {
                char lastChar = t[t.length()];
                if (std::isdigit(static_cast<unsigned char>(lastChar)))
                    sequence.push_back(_symbols_to_sequence(t,symbol_to_id));
                else
                    sequence.resize(N,357);
                sequence_list.push_back(sequence);
                padding_phonemes.push_back(50);
                sequence.clear();
                if (!std::isdigit(static_cast<unsigned char>(lastChar)))
                    sequence.push_back(_symbols_to_sequence(t,symbol_to_id));
            }
        }
        if(sequence.size()<N){
            padding_phonemes.push_back(sequence.size()+1);
            sequence.resize(N,357);
            //如果当前序列全部是填充值357，则都是无效的，不将其添加到序列列表
            bool is_all_zero = std::all_of(
                std::begin(sequence), 
                std::end(sequence), 
                [](int item) { return item == 357; }
            );
            if(!is_all_zero)
                sequence_list.push_back(sequence);
            sequence.clear();
        }

        //循环处理序列列表中的输入，每个输入shape为(1,50)
        for (int k = 0; k < sequence_list.size(); k++)
        {   
            int sum_padding=0;
            vector<float> results;
            vector<float> dim_expend;
            { 
                vector<int> speakers;
                speakers.push_back(0);
                fspeech1.pre_process(speakers,sequence_list[k]);
                fspeech1.inference();
                fspeech1.post_process(results,dim_expend);
            }
            //每个音素值被编码为256维向量，如果当前序列存在填充值，去除输出的填充部分
            if (padding_phonemes[k]<50){
                results.resize(padding_phonemes[k]*256);
                dim_expend.resize(padding_phonemes[k]);
            }
            //对音素发音的持续时间进行重复扩展
            length_outputs length_output;
            length_output = length_regulator(results,dim_expend, padding_phonemes[k], M_pad, L, Dim);
            results.clear();
            dim_expend.clear();
            //使用decoder将encoder的输出在持续时间上扩展后解码为梅尔频谱预测向量，输入shape(1,80,600)
            vector<float> results_2;
            { 
                fspeech2.pre_process(length_output.repeat_encoder_hidden_states);
                fspeech2.inference();
                fspeech2.post_process(results_2,length_output.M_pad);
            }
            //results_2是decoder的输出，输出去除了padding部分，维度为1*80*扩展音素的长度
            std::vector<std::vector<float>> subvectors;
            // 计算有多少个子向量
            int n = (results_2.size()/80) /100; 
            //如果最后一个子向量(1,80,x)不足(1,80,100)，padding到(1,80,100)
            int remaining = (results_2.size()/80)%100; 
            //求results_2中包括多少有效的梅尔频谱预测向量，每个梅尔频谱预测向量为80维
            int size_length=results_2.size()/80;
            //如果不能完整的拆分成若干个（1,80,100）,padding处理
            if(remaining>0){
                //举例说明，results_2:(1,80,155)->result_padding:(1,80,200)->subvectors:两个(1,80,100)，实际数据都被处理成一维的vector
                std::vector<float> result_padding((n+1)*100*80, 0.0f);
                int res_size=(n+1)*100;
                int i=0;
                std::vector<float> output_tmp;
                for(int j=0;j<80;j++){
                    for(int k=0;k<res_size;k++){
                        if(k<size_length){
                            result_padding[j*res_size+k]=results_2[i];
                            i++;
                        }
                    }
                }
                for(int t=0;t<n+1;t++){
                    for(int l=0;l<80;l++){
                        for(int u=0;u<100;u++){
                            output_tmp.push_back(result_padding[t*100+l*res_size+u]);
                        }
                    }
                    subvectors.push_back(output_tmp);
                    output_tmp.clear();
                }
                
            }else{
                //如果results_2可以被准确拆分为若干个(1,80,100)
                std::vector<float> output_tmp;
                for(int t=0;t<n;t++){
                    for(int l=0;l<80;l++){
                        for(int u=0;u<100;u++){
                            output_tmp.push_back(results_2[t*100+l*size_length+u]);
                        }
                    }
                    subvectors.push_back(output_tmp);
                    output_tmp.clear();
                }
            }
            //对每个拆分的梅尔频谱预测数据生成音频
            vector<float> results_mel_one;
            for (int i = 0; i < n+1; i++)
            {
                vector<float> results_mel;
                hifigan.pre_process(subvectors[i]);
                hifigan.inference();
                hifigan.post_process(results_mel);
                //输出result_mel为(1,25600)
                results_mel_one.insert(results_mel_one.end(), results_mel.begin(), results_mel.end());
            }
            if(remaining>0){
                results_mel_one.resize(size_length*256) ;
            }

            results_mel_all.insert(results_mel_all.end(), results_mel_one.begin(), results_mel_one.end());
            results_mel_one.clear();
            subvectors.clear();
        }
        //当所有音频数据生成完毕，保存成wav文件
        VoxUtil::ExportWAV("zh_"+std::to_string(times)+".wav", results_mel_all, 24000);
        results_mel_all.clear();
        times++;
    }
    return 0;
}