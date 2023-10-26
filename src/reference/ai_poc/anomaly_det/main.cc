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
#include "utils.h"
#include "vi_vo.h"
#include "anomaly_det.h"

using std::cerr;
using std::cout;
using std::endl;

std::atomic<bool> isp_stop(false);

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_det> <obj_thres> <input_mode> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_det      异常检测kmodel路径\n"
         << "  obj_thres       异常检测阈值\n"
         << "  input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 5)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[3], "None") == 0)
    {
        // isp not nonsupport yes
    }
    else
    {
        cv::Mat ori_img = cv::imread(argv[3]);
        int ori_w = ori_img.cols;
        int ori_h = ori_img.rows;
        AnomalyDet ad(argv[1], atof(argv[2]), atoi(argv[4]));
        ad.pre_process(ori_img);
        ad.inference();
        vector<anomaly_res> results;
        ad.post_process(results);
        Utils::draw_anomaly_res(ori_img, results);
        cv::imwrite("anomaly_detection_result.jpg", ori_img);
        
    }
    return 0;
}