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

#include "kws.h"


wenet::FeaturePipelineConfig KWS::feature_config = wenet::FeaturePipelineConfig(40, 16000);
wenet::FeaturePipeline KWS::feature_pipeline = wenet::FeaturePipeline(feature_config);

KWS::KWS(const char *kmodel_file, const std::string task_name, const int num_keyword, const float spot_thresh, const int debug_mode)
:task_name(task_name), num_keyword(num_keyword), spot_thresh(spot_thresh), AIBase(kmodel_file,"KWS", debug_mode)
{   
    // kmodel输入
    wav_feature = this -> get_input_tensor(0);
    in_cache = this -> get_input_tensor(1);

    // 根据任务定义 ID与唤醒词 的映射
    if (task_name == "xiaowen")
    {
        idx2res = {"Detected Hi_Xiaowen!", "Detected Nihao_Wenwen", "Deactivated!"};
        std::cout << "当前关键词: 你好问问! Hi 小问!" << std::endl;
    }
    else if (task_name == "commands")
    {
        idx2res = {"Deactivated!", "Detected yes!", "Detected no!", "Detected up!", "Detected down!",
        "Detected left!", "Detected right!", "Detected on!", "Detected off!", "Detected stop!", "Detected go!"};
        std::cout << "当前关键词: commands speech!" << std::endl;
    }
    else if (task_name == "xiaonan")
    {
        idx2res = {"Deactivated!", "Detected xiaonanxiaonan"};
        std::cout << "当前关键词: 小楠小楠!" << std::endl;
    }

    // 第一段音频流无cache可用, 初始化cache为全零
    std::vector<std::vector<float>> cache_init(hidden_dim, std::vector<float>(cache_dim, 0.0f));
    cache = cache_init;
}


KWS::~KWS(){}

// 将音频流数据处理为Kmodel可以接收的数据
bool KWS::pre_process(std::vector<float> wav)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);

    // 根据音频流数据计算音频特征
    feature_pipeline.AcceptWaveform(wav);
    std::vector<std::vector<float>> feats;
    bool ok = feature_pipeline.Read(chunk_size, &feats);

    if (ok) {

        //将feats变为地址连续储存的
        std::vector<float> flattened_feats;
        for (const auto& inner_vector : feats) {
            flattened_feats.insert(flattened_feats.end(), inner_vector.begin(), inner_vector.end());
        }

        std::vector<float> flattened_cache;
        for (const auto& inner_vector_c : cache) {
            flattened_cache.insert(flattened_cache.end(), inner_vector_c.begin(), inner_vector_c.end());
        }

        // 设置kmodel输入
        size_t wav_feature_size = batch_size * chunk_size * num_bin;
        auto buf_1 = wav_feature.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
        memcpy(reinterpret_cast<char *>(buf_1.data()), flattened_feats.data(), wav_feature_size*4);
        size_t in_cache_size = batch_size * hidden_dim * cache_dim;
        auto buf_2 = in_cache.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
        memcpy(reinterpret_cast<char *>(buf_2.data()), flattened_cache.data(), in_cache_size*4);
    }
    return ok;
    
}

// 对音频流进行推理
void KWS::inference()
{   
    this->run();
    this->get_output();
}

// 后处理，输出结果
std::string KWS::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    auto logits = p_outputs_[0];

    //更新cache
    int k = 0;
    for (int i=0; i<cache.size(); i++){
        for (int j=0; j<cache[0].size(); j++){
            cache[i][j] = p_outputs_[1][k];
            k = k + 1;
        }
    }

    // 获得所有帧中每个关键词的最大唤醒概率
    float scores[num_keyword];
    std::fill_n(scores, num_keyword, std::numeric_limits<float>::lowest());

    for (int i = 0; i < 30; i++){
        for (int j = 0; j < num_keyword; j++){
            int index = num_keyword * i + j;
            scores[j] = std::max(scores[j], logits[index]);
        }
    }

    float maxVal = scores[0]; 
    int maxIndex = 0;

    for (int i = 1; i < num_keyword; i++) {
        if (scores[i] > maxVal) {
            maxVal = scores[i];
            maxIndex = i;
        }
    }

    // 大于唤醒阈值则视为唤醒
    if (maxVal > spot_thresh){
        if (num_keyword == 11 && maxIndex == 0){
            return "Deactivated!";
        }
        return idx2res[maxIndex];
    }

    return "Deactivated!";
}



