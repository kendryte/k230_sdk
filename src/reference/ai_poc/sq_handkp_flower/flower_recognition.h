/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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
#ifndef _FLOWER_RECOGNITION_H
#define _FLOWER_RECOGNITION_H

#include <iostream>
#include <vector>

#include "utils.h"
#include "ai_base.h"

/**
 * @brief 花卉识别
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class FlowerRecognition : public AIBase
{
public:
    /**
     * @brief FlowerRecognition构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FlowerRecognition(const char *kmodel_file, const int debug_mode = 1);

    /**
     * @brief FlowerRecognition构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param isp_shape   isp输入大小（chw）
     * @param vaddr       isp对应虚拟地址
     * @param paddr       isp对应物理地址
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    FlowerRecognition(const char *kmodel_file, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode = 1);

    /**
     * @brief FlowerRecognition析构函数
     * @return None
     */
    ~FlowerRecognition();

    /**
     * @brief 图片预处理（ai2d for image）
     * @param ori_img 原始图片
     * @param bbox 花卉在原图上的位置坐标(x,y,w,h)

     * @return None
     */
    void pre_process(cv::Mat ori_img, Bbox &bbox);

    /**
     * @brief 视频流预处理（ai2d for isp）
     * @param bbox 花卉在原图上的位置坐标(x,y,w,h)
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
     * @return 识别结果
     */
    string post_process();

    void softmax(float* x, float* dx, uint32_t len);

    std::vector<std::string> labels
    {
        "pink primrose", "hard-leaved pocket orchid", "snapdragon", "trumpet creeper", "blackberry lily", "colts foot", "king protea", "spear thistle", "yellow iris", "globe-flower", "purple coneflower", "peruvian lily", "balloon flower", "giant white arum lily", "canterbury bells", "fire lily", "pincushion flower", "fritillary", "red ginger", "grape hyacinth", "corn poppy", "prince of wales feathers", "stemless gentian", "artichoke", "sweet william", "sweet pea", "carnation", "garden phlox", "love in the mist", "mexican aster", "alpine sea holly", "ruby-lipped cattleya", "cape flower", "great masterwort", "siam tulip", "lenten rose", "english marigold", "barbeton daisy", "daffodil", "sword lily", "poinsettia", "bolero deep blue", "wallflower", "marigold", "buttercup", "oxeye daisy", "common dandelion", "tiger lily", "petunia", "wild pansy", "primula", "sunflower", "pelargonium", "bishop of llandaff", "gaura", "geranium", "orange dahlia", "pink-yellow dahlia", "moon orchid", "cautleya spicata", "japanese anemone", "black-eyed susan", "silverbush", "californian poppy", "osteospermum", "spring crocus", "bearded iris", "windflower", "tree poppy", "bird of paradise", "gazania", "azalea", "water lily", "rose", "thorn apple", "morning glory", "passion flower", "lotus lotus", "toad lily", "anthurium", "monkshood", "frangipani", "clematis", "hibiscus", "columbine", "desert-rose", "tree mallow", "magnolia", "cyclamen", "watercress", "canna lily", "globe thistle", "hippeastrum", "bee balm", "ball moss", "foxglove", "bougainvillea", "camellia", "mallow", "mexican petunia", "bromelia", "blanket flower"
    };

private:

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    uintptr_t vaddr_;                            // isp的虚拟地址
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
};
#endif
