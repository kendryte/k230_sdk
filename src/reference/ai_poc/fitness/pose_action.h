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

#ifndef _POSE_ACTION_
#define _POSE_ACTION_

#include <vector>
#include "utils.h"

/** 
 * @brief action帮助信息
 */
struct action_helper {
  bool mark = false;  // 是否标记
  int action_count = 0;  // action计数
  int latency = 0;  // 延迟次数
};


/**
 * @brief PoseAction 类
 * 封装了PoseAction类常用的函数，包括清除动作计数函数、获取动作计数函数、获取xy比例函数、获取xyhigher函数、多种动作检查函数等
 */
class PoseAction
{
    public:

        /** 
        * @brief 清除动作计数
        * @param None
        * @return None
        */
        static void clear_action_count();

        /** 
        * @brief 获取动作计数
        * @param recid  某种动作记录编号
        * @return 某动作次数
        */        
        static int get_action_count(int recid);

        /** 
        * @brief 获取xy比例函数
        * @param kpts_sframe  每帧关键点
        * @param index_x  前一个关键点序号
        * @param index_y  后一个关键点序号
        * @return xy比例值
        */  
        static float get_xyratio(std::vector<KKeyPoint> &kpts_sframe, int index_x, int index_y);

        /** 
        * @brief 获取y方向diff
        * @param kpts_sframe  每帧关键点
        * @param index_x  前一个关键点序号
        * @param index_y  后一个关键点序号
        * @return y方向是否diff
        */  
        static bool get_xyhigher(std::vector<KKeyPoint> &kpts_sframe, int index_x, int index_y);

        /** 
        * @brief 获取侧平举次数
        * @param kpts_sframe  每帧关键点
        * @param recid  某种动作记录编号
        * @param thres_conf  关键点阈值
        * @return 侧平举次数
        */  
        static int check_lateral_raise(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf);

        /** 
        * @brief 获取“站立推举”次数
        * @param kpts_sframe  每帧关键点
        * @param recid  某种动作记录编号
        * @param thres_conf  关键点阈值
        * @return 站立推举次数
        */  
        static int check_stand_press(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf);

        /** 
        * @brief 获取“深蹲”次数
        * @param kpts_sframe  每帧关键点
        * @param recid  某种动作记录编号
        * @param thres_conf  关键点阈值
        * @return 深蹲次数
        */        
        static int check_deep_down(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf);

        /** 
        * @brief 获取“深蹲”次数（对xyratio进行了修正）
        * @param kpts_sframe  每帧关键点
        * @param h    修正系数
        * @param recid  某种动作记录编号
        * @param thres_conf  关键点阈值
        * @return 深蹲次数
        */ 
        static int check_deep_down2(std::vector<KKeyPoint> &kpts_sframe, float h, int recid,float thres_conf);

        /** 
        * @brief 单一动作检查
        * @param results_kpts  关键点结果
        * @param h    修正系数
        * @param actionid  动作编号
        * @param thres_conf  关键点阈值
        * @param recid  某种动作记录编号
        * @return 动作次数
        */ 
        static int single_action_check(std::vector<KKeyPoint> &results_kpts, float h, int actionid,float thres_conf, int recid=0);

};

#endif