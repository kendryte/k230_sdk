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
#include "face_detection.h"
#include "face_verification.h"

using std::cout;
using std::endl;

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_det> <det_thres> <nms_thres> <kmodel_recg> <recg_thres> <img_pth_A> <img_pth_B> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_det               人脸检测kmodel路径\n"
         << "  det_thres                人脸检测阈值\n"
         << "  nms_thres                人脸检测nms阈值\n"
         << "  kmodel_recg              人脸识别kmodel路径\n"
         << "  recg_thres               人脸识别阈值\n"
         << "  img_pth_A                本地图片A \n"
         << "  img_pth_B                本地图片B \n"
         << "  debug_mode               是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 9)
    {
        print_usage(argv[0]);
        return -1;
    }

    int debug_mode = atoi(argv[8]);
    FaceDetection face_det(argv[1], atof(argv[2]), atof(argv[3]), debug_mode);
    float recg_thres = atof(argv[5]);
    FaceVerification face_verify(argv[4], debug_mode);

    vector<float> feat_a;
    float score;
    for (int i = 0; i < 2; ++i)
    {
        ScopedTiming st("total time", 1);
        cv::Mat ori_img;
        {
            ScopedTiming st("read image", debug_mode);
            if (i == 0)
                ori_img = cv::imread(argv[6]);
            else
                ori_img = cv::imread(argv[7]);
        }

        // for face det
        face_det.pre_process(ori_img);
        face_det.inference();

        vector<FaceDetectionInfo> det_results;
        face_det.post_process({ori_img.cols, ori_img.rows}, det_results);

        // find max face
        float max_area = 0;
        int max_id = -1;
        for (int i = 0; i < det_results.size(); ++i)
        {
            float area_i = det_results[i].bbox.w * det_results[i].bbox.h;
            if (area_i > max_area)
            {
                max_area = area_i;
                max_id = i;
            }
        }

        // for face verification
        face_verify.pre_process(ori_img, det_results[max_id].sparse_kps.points);
        face_verify.inference();

        if (i == 0)
        {
            face_verify.get_feature(feat_a);
        }
        else
        {
            score = face_verify.calculate_similarity(feat_a);
        }
    }
    if (score > recg_thres)
    {
        cout << "Verification pass : " << score << endl;
    }
    else
    {
        cout << "Verification failed : " << score << endl;
    }

    return 0;
}