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
#ifndef _FACE_MESH_H
#define _FACE_MESH_H

#include <iostream>
#include <vector>

#include "utils.h"
#include "ai_base.h"
#include "face_mesh_post.h"

using std::vector;

#define LOC_SIZE 4
#define CONF_SIZE 2
#define LAND_SIZE 10
#define PI 3.1415926

/**
 * @brief 人脸检测框
 */
typedef struct LeftTopRightBottom
{
    float x0;           // 人脸检测框的左上角x坐标
    float y0;           // 人脸检测框的左上角y坐标
    float x1;           // 人脸检测框的右下角x坐标
    float y1;           // 人脸检测框的右下角y坐标
} LeftTopRightBottom;

/**
 * @brief 人脸Mesh
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class FaceMesh : public AIBase
{
public:
    /**
     * @brief FaceMesh构造函数
     * @param kmodel_file           kmodel文件路径
     * @param post_kmodel_file      后处理kmodel文件路径
     * @param debug_mode            0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceMesh(const char *kmodel_file,const char *post_kmodel_file, const int debug_mode = 1);

    /**
     * @brief FaceMesh构造函数
     * @param kmodel_file           kmodel文件路径
     * @param post_kmodel_file      后处理kmodel文件路径
     * @param isp_shape             isp输入大小
     * @param vaddr                 isp对应虚拟地址
     * @param paddr                 isp对应物理地址
     * @param debug_mode            0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FaceMesh(const char *kmodel_file,const char *post_kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode);

    /**
     * @brief FaceMesh析构函数
     * @return None
     */
    ~FaceMesh();

    /**
     * @brief 图片预处理，（ai2d for image）
     * @param ori_img               原始图片
     * @param bbox                  人脸检测框
     * @return None
     */
    void pre_process(cv::Mat ori_img,Bbox &bbox);

    /**
     * @brief 视频流预处理（ai2d for isp）
     * @param bbox                  人脸检测框
     * @return None
     */
    void pre_process(Bbox &bbox);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @brief kmodel推理结果后处理
     * @param frame_size            原始图像/帧宽高，用于将结果放到原始图像大小
     * @param vertices              后处理后3d点
     * @return None
     */
    void post_process(FrameSize frame_size, vector<float>& vertices,bool pic_mode);

    /**
     * @brief 获取mesh
     * @param img                   展示的图片或图层
     * @param vertices              后处理后3d点
     * @param pic_mode              ture(原图片)，false(osd)
     * @param lcd_mode              ture(hdmi显示)，false(lcd显示)
     * @return None
     */
    void get_mesh(cv::Mat &img, vector<float>& vertices, bool pic_mode,bool lcd_mode=false);
private:

    /**
     * @brief face_mesh常量初始化
     * @return None
     */
    void constant_init();

    /**
     * @brief 根据人脸检测框获取face_alignment roi
     * @param frame_size            原图尺寸，用于防止roi越界
     * @param b                     人脸检测框
     * @param roi_b                 roi(x,y,w,h)
     * @return roi(x1,y1,x2,y2)
     */
    LeftTopRightBottom parse_roi_box_from_bbox(FrameSize frame_size,Bbox& b,Bbox& roi_b);
    
    /**
     * @brief 参数归一化
     * @param                     模型输出参数             
     * @return None
     */
    void param_normalized(vector<float>& param);

    /**
     * @brief 3d关键点渲染
     * @param roi_box_lst          roi(x1,y1,x2,y2)
     * @param vertices             3d关键点              
     * @return None
     */
    void recon_vers(LeftTopRightBottom& roi_box_lst,vector<float> &vertices);

    /**
     * @brief 3d关键点映射，将3d关键点映射roi尺寸
     * @param roi_box              roi(x1,y1,x2,y2)
     * @param vertices             3d关键点              
     * @return None
     */
    void similar_transform(LeftTopRightBottom& roi_box,vector<float> &vertices);

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    uintptr_t vaddr_;                            // isp的虚拟地址
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
    
    FaceMeshPost post_obj_;                      // 人脸mesh后处理对象

    int post_ver_dim_;                           // 3d点个数 
    vector<float> bfm_u;
    vector<float> bfm_w;
    vector<int>   bfm_tri;
    vector<float> ncc_code;
    LeftTopRightBottom roi;                      // roi(x1,y1,x2,y2)
};

#endif