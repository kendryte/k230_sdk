/**
 * @file k_dpu_comm.h
 * @author
 * @brief
 * @version 1.0
 * @date 2023-03-21
 *
 * @copyright
 * Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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
#ifndef __DPU_COMM_H__
#define __DPU_COMM_H__

#include <stdbool.h>
#include "k_errno.h"
#include "k_module.h"
#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     DPU */
/** @{ */ /** <!-- [DPU] */

#define DPU_INT_NUM             (16+170)

#define DPU_MAX_DEV_NUM         (1)
#define DPU_MAX_CHN_NUM         (2)

typedef enum {
    DPU_BIND,                           /**< Binding mode */
    DPU_UNBIND,                         /**< Unbinding mode */
} k_dpu_mode_e;

typedef struct {
    k_u32 start_num;                    /**< Start frame number */
    k_u32 buffer_num;                   /**< Number of DPU buffers */
} k_dpu_init_t;

typedef struct
{
    /* virtual address */
    float* row_start_p0;                                                /**<  粗搜索：TyTz行偏差补偿后起始行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    float* row_end_p0;                                                  /**<  粗搜索：TyTz行偏差补偿后末尾行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    float* row_start_p1;                                                /**<  细搜索：TyTz行偏差补偿后起始行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    float* row_end_p1;                                                  /**<  细搜索：TyTz行偏差补偿后末尾行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    k_u8 *row_start;                                                    /**<  TyTz行偏差补偿后起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u8 *row_end;                                                      /**<  TyTz行偏差补偿后末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    float *row_offset;                                                  /**<  温度行偏差补偿量指针，数组最大长度：1920，取值范围：-32.0~32.0，浮点型：0.02精度 */
    float *col_offset;                                                  /**<  温度列偏差补偿量指针，数组最大长度：1280，取值范围：-32.0~32.0，浮点型：0.02精度 */
    
    k_u8 *row_start_disp;                                               /**< 视差图对齐计算时，每一切块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u8 *row_end_disp;                                                 /**< 视差图对齐计算时，每一切块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u8 *row_start_ir;                                                 /**< 红外图对齐计算时，每一切块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u8 *row_end_ir;                                                   /**< 红外图对齐计算时，每一切块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    float *depth_K;                                                     /**< 深度数值计算参数K指针，数组最大长度：1280，取值范围：500.0~1500000.0，浮点型：0.1精度 */
    float *align_A;                                                     /**< 视差对齐参数A指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    float *align_B;                                                     /**< 视差对齐参数B指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    float *align_C;                                                     /**< 视差对齐参数C指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    float *align_D;                                                     /**< 视差对齐参数D指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    float *align_E;                                                     /**< 视差对齐参数E指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    float *align_F;                                                     /**< 视差对齐参数F指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    float *depth_P;                                                     /**< 深度数值计算参数P指针，数组最大长度：1280，取值范围：5e-7~2e-3，浮点型：1e-8精度 */
    k_u16* block_row_start_ir;                                          /**< 三图对齐：红外图对齐时，每一分块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u16* block_row_end_ir;                                            /**< 三图对齐：红外图对齐时，每一分块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u16* block_row_start_disp;                                        /**< 三图对齐：视差图对齐时，每一分块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u16* block_row_end_disp;                                          /**< 三图对齐：视差图对齐时，每一分块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    float mat_R[3];                                                     /**< 三图对齐：红外相机相对彩色相机旋转矩阵，3*1数组，值域范围：-1.0~1.0，浮点型：0.01精度 */
    float mat_T[3];                                                     /**< 三图对齐：红外相机相对彩色相机平移矩阵，3*1数组，值域范围：-250.0~250.0，浮点型：0.01精度 */

    /* physical address */
    k_u64 row_start_p0_phys;                                            /**<  粗搜索：TyTz行偏差补偿后起始行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    k_u64 row_end_p0_phys;                                              /**<  粗搜索：TyTz行偏差补偿后末尾行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    k_u64 row_start_p1_phys;                                            /**<  细搜索：TyTz行偏差补偿后起始行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    k_u64 row_end_p1_phys;                                              /**<  细搜索：TyTz行偏差补偿后末尾行序号指针，数组最大长度：1920，取值范围：0.0-1920.0，浮点型：0.02精度 */
    k_u64 row_start_phys;                                               /**<  TyTz行偏差补偿后起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 row_end_phys;                                                 /**<  TyTz行偏差补偿后末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 row_offset_phys;                                              /**<  温度行偏差补偿量指针，数组最大长度：1920，取值范围：-32.0~32.0，浮点型：0.02精度 */
    k_u64 col_offset_phys;                                              /**<  温度列偏差补偿量指针，数组最大长度：1280，取值范围：-32.0~32.0，浮点型：0.02精度 */
    
    k_u64 row_start_disp_phys;                                          /**< 视差图对齐计算时，每一切块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 row_end_disp_phys;                                            /**< 视差图对齐计算时，每一切块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 row_start_ir_phys;                                            /**< 红外图对齐计算时，每一切块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 row_end_ir_phys;                                              /**< 红外图对齐计算时，每一切块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 depth_K_phys;                                                 /**< 深度数值计算参数K指针，数组最大长度：1280，取值范围：500.0~1500000.0，浮点型：0.1精度 */
    k_u64 align_A_phys;                                                 /**< 视差对齐参数A指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    k_u64 align_B_phys;                                                 /**< 视差对齐参数B指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    k_u64 align_C_phys;                                                 /**< 视差对齐参数C指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    k_u64 align_D_phys;                                                 /**< 视差对齐参数D指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    k_u64 align_E_phys;                                                 /**< 视差对齐参数E指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    k_u64 align_F_phys;                                                 /**< 视差对齐参数F指针，数组最大长度：1280，取值范围：-10.0~10.0，浮点型：0.001精度 */
    k_u64 depth_P_phys;                                                 /**< 深度数值计算参数P指针，数组最大长度：1280，取值范围：5e-7~2e-3，浮点型：1e-8精度 */
    k_u64 block_row_start_ir_phys;                                      /**< 三图对齐：红外图对齐时，每一分块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 block_row_end_ir_phys;                                        /**< 三图对齐：红外图对齐时，每一分块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 block_row_start_disp_phys;                                    /**< 三图对齐：视差图对齐时，每一分块起始行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 block_row_end_disp_phys;                                      /**< 三图对齐：视差图对齐时，每一分块末尾行序号指针，数组最大长度：1920，取值范围：0-1920，整型 */
    k_u64 mat_R_phys;                                                   /**< 三图对齐：红外相机相对彩色相机旋转矩阵，3*1数组，值域范围：-1.0~1.0，浮点型：0.01精度 */
    k_u64 mat_T_phys;                                                   /**< 三图对齐：红外相机相对彩色相机平移矩阵，3*1数组，值域范围：-250.0~250.0，浮点型：0.01精度 */
} k_dpu_long_parameter_t;

typedef struct {
    bool flag_downScale;                                                /**< 是否粗搜索降分辨率标志位 */
    bool flag_align;                                                    /**< 是否三图对齐标志位 */
    bool flag_align_ir;                                                 /**< 是否红外图对齐标志位 */
    bool flag_align_rgbCoord;                                           /**< 是否视差数值对齐到彩色图坐标系 */
    bool flag_depthout;                                                 /**< 是否输出深度图标志位 */
    bool flag_denoise;                                                  /**< 是否视差图去噪标志位 */
    bool flag_median_p0;                                                /**< 是否粗搜索中值滤波标志位 */
    bool flag_median_denoise;                                           /**< 是否视差去噪内部中值滤波标志位 */
    bool flag_median_post;                                              /**< 是否视差后处理/三图对齐视差图中值滤波标志位 */
    bool flag_median_ir;                                                /**< 是否三图对齐红外图中值滤波标志位 */
    bool flag_check;                                                    /**< 是否物体散斑图质量检测及初始搜索标志位 */

    k_u8 windows_size_lcn;                                              /**< 散斑图LCN处理：窗口大小，取值范围：7~21，整型奇数，典型值为13 */
    k_u8 image_check_window_size;                                       /**< 散斑图质量判定：窗口大小，取值范围：5~51，整型奇数，典型值：35 */
    k_u8 image_check_count_threshold;                                   /**< 散斑图质量判定：散斑点个数阈值；取值范围：1~255，整型，典型值：16 */
    k_u8 image_check_kernel_size;                                       /**< 散斑图质量判定：散斑大小，取值范围：1~11，整型，典型值：5 */
    k_u8 image_check_templ_width;                                       /**< 散斑图质量判定：模板图宽，范围3~37，整型，典型值：9 */
    k_u8 image_check_templ_height;                                      /**< 散斑图质量判定：模板图高，范围3~37，整型，典型值：9 */
    k_u8 windows_size_cost_init0;                                       /**< 初始粗搜索：视差匹配计算窗口大小，取值范围：7~33，整型奇数 */
    k_u8 windows_size_cost_init1;                                       /**< 初始细搜索：视差匹配计算窗口大小，取值范围：7~33，整型奇数 */
    k_u8 windows_size_cost_p0;                                          /**< 粗搜索：视差匹配计算窗口大小，取值范围：7~33，整型奇数 */
    k_u8 windows_size_cost_p1;                                          /**< 细搜索：视差匹配计算窗口大小，取值范围：7~33，整型奇数 */
    k_u8 matching_length_left_init0;                                    /**< 初始粗搜索：视差计算向左搜索范围，取值：0~256，整型 */
    k_u8 matching_length_right_init0;                                   /**< 初始粗搜索：视差计算向右搜索范围，取值：0~256，整型 */
    k_u8 matching_length_left_p0;                                       /**< 粗搜索：视差计算向左搜索范围，取值：0~256，整型 */
    k_u8 matching_length_right_p0;                                      /**< 粗搜索：视差计算向右搜索范围，取值：0~256，整型 */
    k_u8 matching_length_init1;                                         /**< 初始细搜索：视差计算向左/右搜索范围，取值：0~256，整型 */
    k_u8 matching_length_p1;                                            /**< 细搜索：视差计算向左/右搜索范围，取值：0~256，整型 */
    k_u8 row_offset_init0;                                              /**< 初始粗搜索：视差计算上/下搜索范围，取值：0~32，整型 */
    k_u8 row_offset_init1;                                              /**< 初始细搜索：视差计算上/下搜索范围，取值：0~32，整型 */
    k_u8 row_offset_p0;                                                 /**< 粗搜索：视差计算上/下搜索范围，取值：0~32，整型 */
    k_u8 row_offset_p1;                                                 /**< 细搜索：视差计算上/下搜索范围，取值：0~32，整型 */
    k_u8 jump_count_init0;                                              /**< 初始粗搜索：视差计算跳点数，取值范围：2~40，整型 */
    k_u8 jump_count_init1;                                              /**< 初始细搜索：视差计算跳点数，取值范围：2~40，整型 */
    k_u8 jump_count_p0;                                                 /**< 粗搜索：视差计算跳点数，取值范围：2~40，整型 */
    k_u8 jump_count_p1;                                                 /**< 细搜索：视差计算跳点数，取值范围：2~40，整型 */
    k_u8 sub_pixel_type;                                                /**< 匹配搜索：视差图亚像素插值类型序号，0-抛物线插值，1-线性插值，2-直方图插值 */
    k_u8 median_num_thresh_1_p0;                                        /**< 粗搜索：第一次视差图中值滤波-邻域有效点个数阈值，针对3*3中值，取值范围：0-9，整型 */
    k_u8 median_num_thresh_2_p0;                                        /**< 粗搜索：第二次视差图中值滤波-邻域有效点个数阈值，针对3*3中值，取值范围：0-9，整型 */
    k_u8 median_num_thresh_denoise;                                     /**< 视差图去噪：视差图中值滤波-邻域有效点个数阈值，针对3*3中值，取值范围：0-9，整型 */
    k_u8 median_num_thresh_1_post;                                      /**< 视差后处理：第一次视差图中值滤波-邻域有效点个数阈值，针对3*3中值，取值范围：0-9，整型 */
    k_u8 median_num_thresh_2_post;                                      /**< 视差后处理：第二次视差图中值滤波-邻域有效点个数阈值，针对3*3中值，取值范围：0-9，整型 */
    k_u8 median_num_thresh_1_ir;                                        /**< 三图对齐：第一次红外图中值滤波-邻域有效点个数阈值，针对3*3中值，取值范围：0-9，整型 */
    k_u8 median_num_thresh_2_ir;                                        /**< 三图对齐：第二次红外图中值滤波-邻域有效点个数阈值，针对3*3中值，取值范围：0-9，整型 */
    k_u8 median_type_p0;                                                /**< 粗搜索：视差图中值滤波类型，0-普通中值滤波，1-筛选有效点中值滤波 */
    k_u8 median_type_denoise;                                           /**< 视差图去噪：视差图中值滤波类型，0-普通中值滤波，1-筛选有效点中值滤波 */
    k_u8 median_type_post;                                              /**< 视差图后处理：视差图中值滤波类型，0-普通中值滤波，1-筛选有效点中值滤波 */
    k_u8 median_type_ir;                                                /**< 三图对齐：红外图中值滤波类型，0-普通中值滤波，1-筛选有效点中值滤波 */
    k_u8 denoise_down_scale;                                            /**< 视差图去噪：下采样比例，取值：2（降1/2-480P）、4（降1/4-720P）、6（降1/6-720P-跳3/6点）或8（降1/8-1080P） */
    k_u8 upscale_disp;                                                  /**< 三图对齐：视差图对齐—彩色图下采样倍数，取值范围：1~10，整型 */
    k_u8 upscale_ir;                                                    /**< 三图对齐：红外图对齐—彩色图下采样倍数，取值范围：1~10，整型 */
    k_u8 slice_num_h_disp;                                              /**< 三图对齐：视差图对齐时，按照高度方向切块的个数，取值范围：0~256，整型 */
    k_u8 slice_num_h_ir;                                                /**< 三图对齐：红外图对齐时，按照高度方向切块的个数，取值范围：0~256，整型 */

    k_u16 width_speckle;                                                /**< 原始散斑图/红外图宽度像素数，取值范围：0~1280，整型 */
    k_u16 height_speckle;                                               /**< 原始散斑图/红外图高度像素数，取值范围：0~1920，整型 */
    k_u16 width_color;                                                  /**< 原始彩色图宽度像素数，取值范围：0~1280，整型 */
    k_u16 height_color;                                                 /**< 原始彩色图高度像素数，取值范围：0~1440，整型 */
    k_u16 width_output;                                                 /**< 输出视差图/深度图/红外图宽度像素数，取值范围：0~1280，整型 */
    k_u16 height_output;                                                /**< 输出视差图/深度图/红外图高度像素数，取值范围：0~1920，整型 */
    k_u16 area_size_threshold;                                          /**< 视差图去噪：连通域面积阈值，取值范围：0~50000，整型 */
    k_u16 window_size_avg;                                              /**< 三图对齐：红外图对齐时，平均深度值计算窗口大小，取值范围：1~500，整型，典型值：50 */
    k_u16 min_distance;                                                 /**< 三图对齐：最近成像距离（毫米），取值范围：0~65535，整型 */
    k_u16 max_distance;                                                 /**< 三图对齐：最远成像距离（毫米），取值范围：0~65535，整型 */

    float shadow_threshold;                                             /**< 散斑图LCN处理：阴影检测阈值，取值范围：0.0~16.0，浮点型：0.01精度 */
    float image_check_match_threshold;                                  /**< 散斑图质量评定：模板匹配阈值，取值范围：0.0~1.0，浮点型：0.01精度 */
    float min_cost_threshold_init0;                                     /**< 初始粗搜索：最小匹配代价阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float min_conf_threshold_init0;                                     /**< 初始粗搜索：最小置信度阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float min_cost_threshold_init1;                                     /**< 初始细搜索：最小匹配代价阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float min_conf_threshold_init1;                                     /**< 初始细搜索：最小置信度阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float min_cost_threshold_p0;                                        /**< 粗搜索：最小匹配代价阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float min_conf_threshold_p0;                                        /**< 粗搜索：最小置信度阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float min_cost_threshold_p1;                                        /**< 细搜索：最小匹配代价阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float min_conf_threshold_p1;                                        /**< 细搜索：最小置信度阈值，取值范围：0.0~1.0，浮点型：0.001精度 */
    float sub_pixel_k1;                                                 /**< 匹配搜索：视差图亚像素插值系数k1，取值范围：0.0~1.0，浮点型：0.001精度 */
    float sub_pixel_k2;                                                 /**< 匹配搜索：视差图亚像素插值系数k2，取值范围：0.0~1.0，浮点型：0.001精度 */
    float blob_threshold_disp;                                          /**< 视差图去噪：邻域视差之差阈值，取值范围：0.0~32.0，浮点型：0.02精度 */
    float depth_k1;                                                     /**< 三图对齐：深度数值计算系数k1，取值范围：2e-4~1e-2，浮点型：1e-5精度 */
    float depth_k2;                                                     /**< 三图对齐：深度数值计算系数k2，取值范围：0.5~1.5，浮点型：0.001精度 */
    float align_disp;                                                   /**< 三图对齐：视差对齐参数，取值范围：0.0~1500.0，浮点型：0.01精度 */
    float tz;                                                           /**< 三图对齐：投射器z方向偏移量（毫米），取值范围：-5.0~5.0，浮点型：0.001精度 */
    float depth_ref;                                                    /**< 三图对齐：参考图深度值（毫米），取值范围：100.0~5000.0，浮点型：0.1精度 */
    float fx_rgb;                                                       /**< 三图对齐：彩色相机焦距x像素数（对应全分辨率），范围：100.0~5000.0，浮点型：0.1精度 */
    float fy_rgb;                                                       /**< 三图对齐：彩色相机焦距y像素数（对应全分辨率），范围：100.0~5000.0，浮点型：0.1精度 */
    float cx_rgb;                                                       /**< 三图对齐：彩色相机主点x像素坐标（对应全分辨率），范围：0.0~1280.0，浮点型：0.1精度 */
    float cy_rgb;                                                       /**< 三图对齐：彩色相机主点y像素坐标（对应全分辨率），范围：0.0~1440.0，浮点型：0.1精度 */
    float depth_p1;                                                     /**< 视差转深度：深度数值计算系数p1，取值范围：0.0~50000.0，浮点型：0.01精度 */
    float depth_p2;                                                     /**< 视差转深度：深度数值计算系数p2，取值范围：-5.0~100.0，浮点型：0.01精度 */
    float depth_precision;                                              /**< 视差转深度：深度数值精度（倍数），取值范围：0.0~100.0，浮点型：0.1精度 */
} k_dpu_short_parameter_t;

typedef struct
{
    k_dpu_long_parameter_t lpp;         /**< long period parameter */
    k_dpu_short_parameter_t spp;        /**< short period parameter */
} k_dpu_dev_param_t;

typedef struct {
    k_dpu_mode_e mode;                  /**< To configure the DPU mode, including binding and unbinding mode. */
    k_bool tytz_temp_recfg;             /**< 是否更新加载 SAD 温度补偿参数和 TyTz 行补偿计算参数. */
    k_bool align_depth_recfg;           /**< 是否更新加载三图对齐/转深度计算参数. */
    k_bool ir_never_open;               /**< If true, it means that the ir will not be opend during use. */
    k_u32 param_valid;                  /**< Attribute flag of the dpu device. */
    k_dpu_dev_param_t dev_param;        /**< Attribute of the dpu device. */
} k_dpu_dev_attr_t;

typedef struct {
    float depth_k1;                     /**< 直接完全对应一个寄存器。三图对齐：深度数值计算系数k1，取值范围：2e-4~1e-2，浮点型：1e-5精度 */
    float depth_k2;                     /**< 直接完全对应一个寄存器。三图对齐：深度数值计算系数k2，取值范围：0.5~1.5，浮点型：0.001精度 */
    float tz;                           /**< 直接完全对应一个寄存器。三图对齐：投射器z方向偏移量（毫米），取值范围：-5.0~5.0，浮点型：0.001精度 */
} k_dpu_ir_param_t;

typedef struct {
    k_u8 param_valid;                   /**< Attribute flag of infrared image channel. */
    k_s32 chn_num;                      /**< Channel number selected by the infrared image. */
    k_dpu_ir_param_t ir_param;          /**< Attribute of the infrared image. */
} k_dpu_chn_ir_attr_t;

typedef struct {
    k_u8 matching_length_left_p0;       /**< 粗搜索：视差计算向左搜索范围，取值：0~256，整型 */
    k_u8 matching_length_right_p0;      /**< 粗搜索：视差计算向右搜索范围，取值：0~256，整型 */

    float image_check_match_threshold;  /**< 散斑图质量评定：模板匹配阈值，取值范围：0.0~1.0，浮点型：0.01精度 */
    float depth_p1;                     /**< 视差转深度：深度数值计算系数p1，取值范围：0.0~50000.0，浮点型：0.01精度 */
    float depth_p2;                     /**< 视差转深度：深度数值计算系数p2，取值范围：-5.0~100.0，浮点型：0.01精度 */
    float depth_precision;              /**< 视差转深度：深度数值精度（倍数），取值范围：0.0~100.0，浮点型：0.1精度 */
} k_dpu_lcn_param_t;

typedef struct {
    k_u8 param_valid;                   /**< Attribute flag of speckle image channel. */
    k_s32 chn_num;                      /**< Channel number selected by the speckle image. */
    k_dpu_lcn_param_t lcn_param;        /**< Attribute of the speckle image. */
} k_dpu_chn_lcn_attr_t;

typedef struct {
    k_bool valid;                       /**< Flag used to indicate whether a disparity output is vaild. */
    k_u32 length;                       /**< Length of the disparity image. */
    k_u64 disp_phys_addr;               /**< Physical address of disparity image. */
    k_u64 disp_virt_addr;               /**< Virtual address of disparity image. */
} k_dpu_disp_out_t;

typedef struct {
    k_bool valid;                       /**< Flag used to indicate whether a depth output is valid. */
    k_u32 length;                       /**< Length of the depth image. */
    k_u64 depth_phys_addr;              /**< Physical address of depth image. */
    k_u64 depth_virt_addr;              /**< Virtual address of depth image. */
} k_dpu_depth_out_t;

typedef struct {
    k_bool valid;                       /**< Flag used to indicate whether a infrared output is valid. */
    k_u32 length;                       /**< Length of the infrared image. */
    k_u64 ir_phys_addr;                 /**< Physical address of infrared image. */
    k_u64 ir_virt_addr;                 /**< Virtual address of infrared image. */
} k_dpu_ir_out_t;

typedef struct {
    k_bool valid;                       /**< Flag used to indicate whether quality check output is valid. */
    k_u32 qlt_length;                   /**< Length of the quality check. */
    k_u32 sad_disp_length;              /**< Length of the sad disp. */
    k_u32 init_sad_disp_length;         /**< Length of the init sad disp. */
    k_u64 qlt_phys_addr;                /**< 输出质量检测结果的图像 mask 的物理地址. */
    k_u64 qlt_virt_addr;                /**< 输出质量检测结果的图像 mask 的虚拟地址. */
    k_u64 sad_disp_phys_addr;           /**< 粗细_列视差（初始分辨率）输出的初始地址，物理地址. */
    k_u64 sad_disp_virt_addr;           /**< 粗细_列视差（初始分辨率）输出的初始地址，虚拟地址. */
    k_u64 init_sad_disp_phys_addr;      /**< 初始行列视差输出的初始地址（行列xy拼接保存），物理地址. */
    k_u64 init_sad_disp_virt_addr;      /**< 初始行列式差输出的初始地址（行列xy拼接保存），虚拟地址. */
} k_dpu_qlt_out_t;

typedef struct {
    k_u8 dev_flag;                      /**< Attribute flag of the dpu device. */
    k_u8 lcn_flag;                      /**< Attribute flag of speckle image channel. */
    k_u8 ir_flag;                       /**< Attribute flag of infrared image channel. */
} k_dpu_param_flag_t;

typedef struct {
    k_u64 pool_phys_addr;
    k_u32 pool_id;
    k_u32 time_ref;                     /**< Transparent frame number */
    k_u64 pts;                          /**< Tims stamp. */
    k_dpu_disp_out_t disp_out;          /**< Structure of disparity image output result. */
    k_dpu_depth_out_t depth_out;        /**< Structure of depth image output result. */
    k_dpu_qlt_out_t qlt_out;            /**< Structure of quality output result. */
    k_dpu_param_flag_t flag;            /**< Structure of the attribute flag. */
} k_dpu_chn_lcn_result_t;

typedef struct {
    k_u64 pool_phys_addr;
    k_u32 pool_id;
    k_u32 time_ref;                     /**< Transparent frame number */
    k_u64 pts;                          /**< Time stamp. */
    k_dpu_ir_out_t ir_out;              /**< Structure of infrared image output result. */
    k_dpu_param_flag_t flag;            /**< Structure of the attribute flag. */
} k_dpu_chn_ir_result_t;

typedef union {
    k_dpu_chn_lcn_result_t lcn_result;  /**< Result of the speckle channel. */
    k_dpu_chn_ir_result_t ir_result;    /**< Result of the infrared channel. */
} k_dpu_chn_result_u;

typedef struct {
    k_bool used;                        /**< Mark whether the space has been alloced. */
    k_u32 size;                         /**< Length of the alloced space. */
    k_u64 phys_addr;                    /**< Physical address. */
    void *virt_addr;                    /**< Virtual address. */
} k_dpu_user_space_t;

#define K_ERR_DPU_INVALID_DEVID     K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_INVALID_DEVID)
#define K_ERR_DPU_INVALID_CHNID     K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_INVALID_CHNID)
#define K_ERR_DPU_ILLEGAL_PARAM     K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_ILLEGAL_PARAM)
#define K_ERR_DPU_EXIST             K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_EXIST)
#define K_ERR_DPU_UNEXIST           K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_UNEXIST)
#define K_ERR_DPU_NULL_PTR          K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_NULL_PTR)
#define K_ERR_DPU_NOT_CONFIG        K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOT_CONFIG)
#define K_ERR_DPU_NOT_SUPPORT       K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOT_SUPPORT)
#define K_ERR_DPU_NOT_PERM          K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOT_PERM)
#define K_ERR_DPU_NOMEM             K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOMEM)
#define K_ERR_DPU_NOBUF             K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOBUF)
#define K_ERR_DPU_BUF_EMPTY         K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_BUF_EMPTY)
#define K_ERR_DPU_BUF_FULL          K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_BUF_FULL)
#define K_ERR_DPU_NOTREADY          K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_NOTREADY)
#define K_ERR_DPU_BADADDR           K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_BADADDR)
#define K_ERR_DPU_BUSY              K_DEF_ERR(K_ID_DPU, K_ERR_LEVEL_ERROR, K_ERR_BUSY)

/** @} */ /** <!-- ==== DPU End ==== */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif