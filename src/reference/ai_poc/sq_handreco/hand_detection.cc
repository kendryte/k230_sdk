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
#include "hand_detection.h"
#include "k_autoconf_comm.h"
#include <vector>

HandDetection::HandDetection(const char *kmodel_file, float obj_thresh, float nms_thresh, FrameSize frame_size,const int debug_mode)
:obj_thresh_(obj_thresh), nms_thresh_(nms_thresh),frame_size_(frame_size),AIBase(kmodel_file, "HandDetection", debug_mode)
{
    model_name_ = "HandDetection";
    classes_num_ = 1;
    int net_len = input_shapes_[0][2];
    ai2d_out_tensor_ = get_input_tensor(0);
}

HandDetection::HandDetection(const char *kmodel_file, float obj_thresh, float nms_thresh, FrameSize frame_size, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
: obj_thresh_(obj_thresh), nms_thresh_(nms_thresh),frame_size_(frame_size), AIBase(kmodel_file,"HandDetection", debug_mode)
{
    model_name_ = "HandDetection";
    classes_num_ = 1;
    vaddr_ = vaddr;
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    int isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;
#if 0
    int in_size = isp_shape.channel * isp_shape.height * isp_shape.width;
    ai2d_in_tensor_ = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, in_size },
        true, hrt::pool_shared).expect("cannot create input tensor");
#else
    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
#endif
    ai2d_out_tensor_ = get_input_tensor(0);
    Utils::padding_resize(isp_shape, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
}


HandDetection::~HandDetection()
{
}

// ai2d for image
void HandDetection::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> rgb_chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, rgb_chw_vec);
    Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, rgb_chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
}

// ai2d for video
void HandDetection::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
#if 0
    ai2d_builder_->invoke(ai2d_in_tensor_, ai2d_out_tensor_).expect("error occurred in ai2d running");
#else
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_, ai2d_out_tensor_).expect("error occurred in ai2d running");    
#endif
}

void HandDetection::inference()
{
    this->run();
    this->get_output();
}

void HandDetection::post_process(std::vector<BoxInfo> &result)
{
    auto boxes0 = decode_infer(p_outputs_[0], 8, frame_size_, anchors_0);
    result.insert(result.begin(), boxes0.begin(), boxes0.end());
    auto boxes1 = decode_infer(p_outputs_[1], 16, frame_size_, anchors_1);
    result.insert(result.begin(), boxes1.begin(), boxes1.end());
    auto boxes2 = decode_infer(p_outputs_[2], 32, frame_size_, anchors_2);
    result.insert(result.begin(), boxes2.begin(), boxes2.end());
    nms(result);
}

void HandDetection::nms(std::vector<BoxInfo> &input_boxes)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
            * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        for (int j = i + 1; j < int(input_boxes.size());)
        {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= nms_thresh_)
            {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else
            {
                j++;
            }
        }
    }
}

std::vector<BoxInfo> HandDetection::decode_infer(float *data, int stride, FrameSize frame_size, float anchors[][2])
{
//std::cout << "decode_infer!\n";
    float ratiow = (float)input_shapes_[0][3] / frame_size.width;
    float ratioh = (float)input_shapes_[0][2] / frame_size.height;
    float gain = ratiow < ratioh ? ratiow : ratioh;
    std::vector<BoxInfo> result;
    int grid_size = input_shapes_[0][2] / stride;
    int one_rsize = classes_num_ + 5;
    float cx, cy, w, h;
    for (int shift_y = 0; shift_y < grid_size; shift_y++)
    {
        for (int shift_x = 0; shift_x < grid_size; shift_x++)
        {
            int loc = shift_x + shift_y * grid_size;
            for (int i = 0; i < 3; i++)
            {
                float *record = data + (loc * 3 + i) * one_rsize;
                float *cls_ptr = record + 5;
                for (int cls = 0; cls < classes_num_; cls++)
                {
                    // float score = sigmoid(cls_ptr[cls]) * sigmoid(record[4]);
                    float score = cls_ptr[cls] * record[4];
                    if (score > obj_thresh_)
                    {
                        cx = (record[0] * 2.f - 0.5f + (float)shift_x) * (float)stride;
                        cy = (record[1] * 2.f - 0.5f + (float)shift_y) * (float)stride;
                        w = pow(record[2] * 2.f, 2) * anchors[i][0];
                        h = pow(record[3] * 2.f, 2) * anchors[i][1];

                        cx -= ((input_shapes_[0][3] - frame_size.width * gain) / 2);
                        cy -= ((input_shapes_[0][2] - frame_size.height * gain) / 2);
                        cx /= gain;
                        cy /= gain;
                        w /= gain;
                        h /= gain;
                        BoxInfo box;
                        box.x1 = std::max(0, std::min(int(frame_size.width), int(cx - w / 2.f)));
                        box.y1 = std::max(0, std::min(int(frame_size.height), int(cy - h / 2.f)));
                        box.x2 = std::max(0, std::min(int(frame_size.width), int(cx + w / 2.f)));
                        box.y2 = std::max(0, std::min(int(frame_size.height), int(cy + h / 2.f)));
                        
                        #if defined(CONFIG_BOARD_K230_CANMV) || defined(CONFIG_BOARD_K230_CANMV_V2)
                            if (abs(box.y1-box.y2)< 0.1*frame_size.height)
                                continue;
                            if ((abs(box.x1-box.x2)< 0.25*frame_size.width) && ((box.x1 < 0.03*frame_size.width) || (box.x2 > 0.97*frame_size.width))) 
                                continue;
                            if ((abs(box.x1-box.x2)< 0.15*frame_size.width) && ((box.x1 < 0.01*frame_size.width) || (box.x2 > 0.99*frame_size.width)))
                                continue;
                        #else
                            if ((abs(box.x1-box.x2)< 0.1*frame_size.width) || (abs(box.y1-box.y2)< 0.1*frame_size.height))
                                continue;
                        #endif

                        box.score = score;
                        box.label = cls;
                        result.push_back(box);
                    }
                }
            }
        }
    }
    return result;
}
