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
#include "face_mesh.h"
#include "k230_math.h"
#include "constant.h"

// for image
FaceMesh::FaceMesh(const char *kmodel_file,const char *post_kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FaceMesh", debug_mode),post_obj_(post_kmodel_file,debug_mode)
{
    model_name_ = "FaceMesh";
    ai2d_out_tensor_ = get_input_tensor(0);
    post_ver_dim_ = post_obj_.output_shapes_[0][1];
}

// for video
FaceMesh::FaceMesh(const char *kmodel_file,const char *post_kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode) : AIBase(kmodel_file,"FaceMesh", debug_mode),post_obj_(post_kmodel_file,debug_mode)
{
    model_name_ = "FaceMesh";
    post_ver_dim_ = post_obj_.output_shapes_[0][1];
    vaddr_ = vaddr;

    // ai2d_in_tensor to isp
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    int isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;
#if 0
    ai2d_in_tensor_ = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, isp_size },
        true, hrt::pool_shared).expect("cannot create input tensor");
#else
    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
#endif

    ai2d_out_tensor_ = get_input_tensor(0);
}

// ai2d for image
void FaceMesh::pre_process(cv::Mat ori_img,Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    
    Bbox roi_b;
    roi = parse_roi_box_from_bbox({ori_img.cols,ori_img.rows},bbox,roi_b);
    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img,chw_vec);
    Utils::crop_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, roi_b, ai2d_out_tensor_);
}

// ai2d for video
void FaceMesh::pre_process(Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");

    Bbox roi_b;
    roi = parse_roi_box_from_bbox({isp_shape_.width,isp_shape_.height},bbox,roi_b);
    Utils::crop_resize(roi_b,ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}

void FaceMesh::inference()
{
    this->run();
    this->get_output();
}

void FaceMesh::post_process(FrameSize frame_size,vector<float>& vertices,bool pic_mode)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    vector<float> param(p_outputs_[0],p_outputs_[0] + output_shapes_[0][1]);
    param_normalized(param);

    post_obj_.pre_process(param);
    post_obj_.inference();
    post_obj_.post_process(vertices);
    
    if(pic_mode==false)
    {
        //因为要在osd上画图，故需要把返回值变换到osd的长、宽
        roi.x0 = roi.x0 / isp_shape_.width * frame_size.width;
        roi.y0 = roi.y0 / isp_shape_.height * frame_size.height;
        roi.x1 = roi.x1 / isp_shape_.width * frame_size.width;
        roi.y1 = roi.y1 / isp_shape_.height * frame_size.height; 
    }   
    recon_vers(roi, vertices);
}

void FaceMesh::get_mesh(cv::Mat &src_img, vector<float>& vertices,bool pic_mode,bool lcd_mode)
{
    int x,y;
    if (lcd_mode)
    {
        for(int ver_index =0;ver_index < post_ver_dim_;ver_index = ver_index + 12)
        {
            x = vertices[ver_index];
            y = vertices[ver_index + post_ver_dim_];
            if(pic_mode)
                cv::circle(src_img, cv::Point(x, y), 1, cv::Scalar(255, 0, 0), 1);  
            else
                cv::circle(src_img, cv::Point(x, y), 2, cv::Scalar(200, 0, 0, 255), 4); 
        }
    }
    else
    {
        for(int ver_index =0;ver_index < post_ver_dim_;ver_index = ver_index + 6)
        {
            x = vertices[ver_index];
            y = vertices[ver_index + post_ver_dim_];
            if(pic_mode)
                cv::circle(src_img, cv::Point(x, y), 1, cv::Scalar(255, 0, 0), 1);  
            else
                cv::circle(src_img, cv::Point(x, y), 2, cv::Scalar(200, 0, 0, 255), 4); 
        }
    }
}

/*************************常量操作********************/
void FaceMesh::constant_init()
{
    bfm_tri = Utils::read_binary_file<int>("bfm_tri.bin");
    ncc_code = Utils::read_binary_file<float>("ncc_code.bin");
}
/*************************前处理********************/
LeftTopRightBottom FaceMesh::parse_roi_box_from_bbox(FrameSize frame_size,Bbox& b,Bbox& roi_b)
{
    float old_size = ( b.w + b.h) / 2;
    float center_x = b.x + b.w / 2;
    float center_y = b.y + b.h / 2 + old_size * 0.14;
    int size = int(old_size * 1.58);
    
    float x0 = center_x - float(size) / 2;
    float y0 = center_y - float(size) / 2;
    float x1 = x0 + size;
    float y1 = y0 + size;

    x0 = std::max(float(0), std::min(x0, float(frame_size.width)));
    y0 = std::max(float(0), std::min(y0, float(frame_size.height)));
    x1 = std::max(float(0), std::min(x1, float(frame_size.width)));
    y1 = std::max(float(0), std::min(y1, float(frame_size.height)));

    roi_b.x = x0;
    roi_b.y = y0;
    roi_b.w = x1 - x0;
    roi_b.h = y1 - y0;
    return {x0,y0,x1,y1};
}

/*************************后处理********************/

void FaceMesh::param_normalized(vector<float>& param)
{
    for (int param_index = 0; param_index < param.size(); ++param_index)
    {
        param[param_index] = param[param_index] * param_std[param_index] + param_mean[param_index];
    }
}

void FaceMesh::recon_vers(LeftTopRightBottom& roi_box_lst,vector<float> &vertices)
{
    similar_transform(roi_box_lst,vertices);
}

void FaceMesh::similar_transform(LeftTopRightBottom& roi_box,vector<float> &vertices)
{
    double scale_x = (roi_box.x1 - roi_box.x0) / input_shapes_[0][3];
    double scale_y = (roi_box.y1 - roi_box.y0) / input_shapes_[0][3];
    double s = (scale_x + scale_y) / 2;

    for (int row_index = 0; row_index < 3; ++row_index)
    {
        if (row_index == 0)
        {
            int index = 0;
            for (int col_index = 0; col_index < post_ver_dim_; ++col_index)
            {
                index = row_index * post_ver_dim_ + col_index;
                vertices[index] -= 1;
                vertices[index] = vertices[index] * scale_x + roi_box.x0;
            }
        }
        else if (row_index == 2)
        {
            int index = 0;
            float min_dim2 = 0;
            for (int col_index = 0; col_index < post_ver_dim_; ++col_index)
            {
                index = row_index * post_ver_dim_ + col_index;
                vertices[index] -= 1;
                vertices[index] *= s;
                if (col_index == 0)
                    min_dim2 = vertices[index];
                else
                {
                    if (vertices[index] < min_dim2)
                        min_dim2 = vertices[index];
                }
            }
            for (int col_index = 0; col_index < post_ver_dim_; ++col_index)
            {
                vertices[row_index * post_ver_dim_ + col_index] -= min_dim2;
            }
        }
        else
        {
            int index = 0;
            for (int col_index = 0; col_index < post_ver_dim_; ++col_index)
            {
                index = row_index * post_ver_dim_ + col_index;
                vertices[index] = input_shapes_[0][3] - vertices[index];
                vertices[index] = vertices[index] * scale_y + roi_box.y0;
            }
        }
    }
}

FaceMesh::~FaceMesh()
{
}