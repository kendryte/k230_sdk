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
    // assert(n == rhs.size());
    float tmp = 0.0;  //内积
    for (int i = 0; i<n; ++i)
        tmp += lhs[i] * rhs[i];
    return tmp / (getMold(lhs)*getMold(rhs));
}

bool endsWith(const std::string& str, const std::string suffix) {
    if (suffix.length() > str.length()) 
    { 
        return false; 
    }

    return (str.rfind(suffix) == (str.length() - suffix.length()));
} 

void GetFileNames(string path,vector<string>& filenames)
{
    DIR *pDir;
    struct dirent* ptr;
    if(!(pDir = opendir(path.c_str()))){
        cout <<"Folder doesn't Exist!"<< endl;
        return;
    }
    while((ptr = readdir(pDir))!=0) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0){
            filenames.push_back(path + "/" + ptr->d_name);
    }
    }
    closedir(pDir);
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


// for image
SL::SL(const char *kmodel_file, const int debug_mode): AIBase(kmodel_file,"self_learning", debug_mode)
{

    model_name_ = "self_learning";
    
    ai2d_out_tensor_ = get_input_tensor(0);

}


// for video
SL::SL(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode):AIBase(kmodel_file,"self_learning", debug_mode)
{
    model_name_ = "self_learning";
    
    vaddr_ = vaddr;

    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape_.channel, isp_shape_.height, isp_shape_.width};
    int isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    #if 0
    ai2d_in_tensor_ = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, isp_size },
        true, hrt::pool_shared).expect("cannot create input tensor");
    #else
    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
    #endif

    // ai2d_out_tensor
    ai2d_out_tensor_ = get_input_tensor(0);
    // fixed padding resize param
    Utils::resize(ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}


SL::~SL()
{

}

// ai2d for image
void SL::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);

    cv::cvtColor( ori_img, ori_img,cv::COLOR_BGR2RGB);

    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, ai2d_out_tensor_);
}

// ai2d for video
void SL::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    #if 0
    ai2d_builder_->invoke().expect("error occurred in ai2d running");
    #else
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_,ai2d_out_tensor_).expect("error occurred in ai2d running");
    // run ai2d
    #endif
}

void SL::inference()
{
    ScopedTiming st(model_name_ + " inference", debug_mode_);
    this->run();
    this->get_output();
}

void SL::post_process( vector<Evec>& results,int topK)
{

    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *output = p_outputs_[0];

    int length = output_shapes_[0][1];
    vector<float> output_vec(output, output + length);

    // Utils::dump_binary_file("onboard.bin", (char *)output, length * sizeof(float));

    // float output_mold = getMold(output_vec);

    const char* filePath = "vectors";

    vector<string> dirs;
    GetFileNames(filePath,dirs);

    vector<string> files;
    for( auto dir:dirs )
    {
        GetFileNames(dir,files);
    }

    for(auto file:files)
    {
        if( endsWith( file, ".bin") )
        {
            vector<float> vec = Utils::read_binary_file< float >(file.c_str());

            float score = getSimilarity(output_vec, vec);

            if( results.size() < topK )
            {
                Evec evec;
                vector<string> res = split( file, "/" );
                evec.category = res[1];
                evec.score = score;
                evec.bin_file = file;
                results.push_back( evec );
                std::sort(results.begin(), results.end(), GreaterSort);

            }
            else
            {
                if( score <= results[topK-1].score )
                {
                    continue;
                }
                else
                {
                    Evec evec;
                    vector<string> res = split( file, "/" );
                    evec.category = res[1];
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

float* SL::get_vecOutput( )
{
    return p_outputs_[0]  ;
}

int SL::get_len()
{
    int length = output_shapes_[0][1];
    return length;
}