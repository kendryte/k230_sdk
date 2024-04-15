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

#include "pose_detect.h"
#include "utils.h"


// for image
poseDetect::poseDetect(const char *kmodel_file, float obj_thresh,float nms_thresh,  const int debug_mode): obj_thresh_(obj_thresh),nms_thresh_(nms_thresh), AIBase(kmodel_file,"poseDetect", debug_mode)
{
    model_name_ = "poseDetect";
    
    ai2d_out_tensor_ = get_input_tensor(0);
}   

// for video
poseDetect::poseDetect(const char *kmodel_file, float obj_thresh,float nms_thresh, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode): obj_thresh_(obj_thresh),nms_thresh_(nms_thresh), AIBase(kmodel_file,"poseDetect", debug_mode)
{
    model_name_ = "poseDetect";
    
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
    Utils::padding_resize_params(params,isp_shape_, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
}

poseDetect::~poseDetect()
{

}

// ai2d for image
void poseDetect::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::padding_resize_params(params,{ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
    
}

// ai2d for video
void poseDetect::pre_process()
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

void poseDetect::inference()
{
    this->run();
    this->get_output();
}

bool poseDetect::BatchDetect(  float* all_data, std::vector<std::vector<OutputPose>>& output,cv::Vec4d params)
{

    _outputTensorShape = { output_shapes_[0][0], output_shapes_[0][1], output_shapes_[0][2] };
    _anchorLength = output_shapes_[0][1];

    // [1, 56 ,8400] -> [1, 8400, 56]
    cv::Mat output0 = cv::Mat(cv::Size((int)_outputTensorShape[2], (int)_outputTensorShape[1]), CV_32F, all_data).t(); 
 
    float* pdata = (float*)output0.data; // [classid,x,y,w,h,x,y,...21个点]
    int rows = output0.rows; // 预测框的数量 8400
    // 一张图片的预测框
 
    vector<float> confidences;
    vector<Rect> boxes;
    vector<int> labels;
    vector<vector<float>> kpss;

    for (int r=0; r<rows; ++r){
 
        // 得到人类别概率
        auto kps_ptr = pdata + 5;
 
        // 预测框坐标映射到原图上
        float score = pdata[4];
        
        if(score > obj_thresh_){
            float x = (pdata[0] - params[2]) / params[0]; //x
            float y = (pdata[1] - params[3]) / params[1]; //y
            float w = pdata[2] / params[0]; //w
            float h = pdata[3] / params[1]; //h
 
            // int left = MAX(int(x - 0.5 *w +0.5), 0);
            // int top = MAX(int(y - 0.5*h + 0.5), 0);
            float left = MAX(int(x - 0.5 *w +0.5), 0);
            float top = MAX(int(y - 0.5*h + 0.5), 0);
 
            std::vector<float> kps;
            for (int k=0; k< 17; k++){
                
                float kps_x = (*(kps_ptr + 3 * k) - params[2]) / params[0];
                float kps_y = (*(kps_ptr + 3 * k + 1) - params[3]) / params[1];
                float kps_s = *(kps_ptr + 3 * k + 2);
 
                kps.push_back(kps_x);
                kps.push_back(kps_y);
                kps.push_back(kps_s);
            }
 
            confidences.push_back(score);
            labels.push_back(0);
            kpss.push_back(kps);
            // boxes.push_back(Rect(left, top, int(w + 0.5), int(h + 0.5)));
            boxes.push_back(Rect(left, top, float(w + 0.5), float(h + 0.5)));
        }
        pdata += _anchorLength; //下一个预测框
    }
 
    // 对一张图的预测框执行NMS处理
    vector<int> nms_result;
    
    std::vector<BoxInfo> boxinfo_results;
    BoxInfo res;
    float x1,y1,x2,y2,score_;
    int label,idx;
    for(int i=0;i<boxes.size();i++)
    {
        x1 =  float( boxes[i].tl().x ) ;
        y1 = float( boxes[i].tl().y ) ;
        x2 = float( boxes[i].br().x ) ;
        y2 = float( boxes[i].br().y ) ;
        score_ = confidences[i];
        label = labels[i];
        idx = i;

        res = {  x1,y1,x2,y2,score_,label,idx };
        boxinfo_results.push_back(res);
    }

    Utils::nms_pose(boxinfo_results, nms_thresh_,nms_result);

    // 对一张图片：依据NMS处理得到的索引，得到类别id、confidence、box，并置于结构体OutputDet的容器中
    vector<OutputPose> temp_output;
    for (size_t i=0; i<boxinfo_results.size(); ++i){
        int idx = boxinfo_results[i].idx;
        OutputPose result;
 
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        result.label = labels[idx];
        result.kps = kpss[idx];

        temp_output.push_back(result);
    }
    output.push_back(temp_output); // 多张图片的输出；添加一张图片的输出置于此容器中
 
    if (output.size())
        return true;
    else
        return false;
 
}

bool poseDetect::Detect(float* all_data, std::vector<OutputPose> &output,cv::Vec4d params){
    vector<vector<OutputPose>> temp_output;

    bool flag = BatchDetect(all_data, temp_output,params);
    output = temp_output[0];
    return true;
}


bool poseDetect::post_process( std::vector<OutputPose> &output,cv::Vec4d params)
{

    ScopedTiming st(model_name_ + " post_process video", debug_mode_);
    foutput_0 = p_outputs_[0];
    bool find_ = Detect(foutput_0,output,params);

    return find_;

}

