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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.f
 */

#include "self_learning.h"

bool GreaterSort(Evec a, Evec b) // 降序
{
    return (a.score > b.score);
}
bool LessSort(Evec a, Evec b)  // 升序
{
    return (a.score < b.score);
}

float getMold(const vector<float>& vec){   //求向量的模长
    int n = vec.size();
    float sum = 0.0;
    for (int i = 0; i<n; ++i)
        sum += vec[i] * vec[i];
    return sqrt(sum);
}

float getSimilarity(const vector<float>& lhs, const vector<float>& rhs){
    int n = lhs.size();

    float tmp = 0.0;  //内积
    for (int i = 0; i<n; ++i)
    {
        tmp += lhs[i] * rhs[i];
    }

    return tmp / (getMold(lhs)*getMold(rhs));
}

bool endsWith(const std::string& str, const std::string suffix) {
    if (suffix.length() > str.length()) 
    { 
        return false; 
    }

    return (str.rfind(suffix) == (str.length() - suffix.length()));
} 

vector<string> split(const string &str, const string &pattern)
{
    char * strc = new char[strlen(str.c_str())+1];
    strcpy(strc, str.c_str());   //string转换成C-string
    vector<string> res;
    char* temp = strtok(strc, pattern.c_str());
    while(temp != NULL)
    {
        res.push_back(string(temp));
        temp = strtok(NULL, pattern.c_str());
    }
    delete[] strc;
    return res;
}


SelfLearning::SelfLearning(const char *kmodel_file, FrameSize crop_wh, float thres, int topk, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
:topk(topk), thres(thres), AIBase(kmodel_file,"self_learning", debug_mode)
{
    vaddr_ = vaddr;
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape_.channel, isp_shape_.height, isp_shape_.width};

    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
    ai2d_out_tensor_ = get_input_tensor(0);

    crop_box.w = float(crop_wh.width);
    crop_box.h = float(crop_wh.height);
    crop_box.x = (isp_shape.width / 2.0) - (crop_box.w / 2.0);
    crop_box.y = (isp_shape.height / 2.0) - (crop_box.h / 2.0);

    Utils::crop_resize(crop_box, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}

SelfLearning::~SelfLearning()
{

}

void SelfLearning::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);

    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();

    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);

    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_,ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void SelfLearning::inference()
{
    this->run();
    this->get_output();
}

float* SelfLearning::get_kpu_output(int *out_len)
{
    *out_len = output_shapes_[0][1];
    return p_outputs_[0];
}

void SelfLearning::post_process(std::vector<std::string> features, std::vector<Evec> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    float *output = p_outputs_[0];
    int length = output_shapes_[0][1];

    vector<float> output_vec(output, output + length);
    for(auto file:features)
    {
        if( endsWith( file, ".bin") )
        {
            vector<float> vec = Utils::read_binary_file< float >(("features/" + file).c_str());
            float score = getSimilarity(output_vec, vec);
            if (score > thres)
            {
                vector<string> res = split( file, "_" );
                bool is_same = false;
                for (auto r: results)
                {
                    if (r.category ==  res[0])
                    {
                        if (r.score < score)
                        {
                            r.bin_file = file;
                            r.score = score;
                        }
                        is_same = true;
                    }
                }
                
                if (!is_same)
                {
                    if( results.size() < topk)
                    {
                        Evec evec;
                        evec.category = res[0];
                        evec.score = score;
                        evec.bin_file = file;
                        results.push_back( evec );
                        std::sort(results.begin(), results.end(), GreaterSort);

                    }
                    else
                    {
                        if( score <= results[topk-1].score )
                        {
                            continue;
                        }
                        else
                        {
                            Evec evec;
                            evec.category = res[0];
                            evec.score = score;
                            evec.bin_file = file;
                            results.push_back( evec );
                            std::sort(results.begin(), results.end(), GreaterSort);
                            
                            results.pop_back();
                        }
                    }
                }
            }
        }
    }
}

