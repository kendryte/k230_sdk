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
#ifndef _KEY_H
#define _KEY_H

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>

#include <opencv2/opencv.hpp>


/**
 * @brief 键盘按钮
 * 主要封装了对于每一个键盘按钮的处理
 */
class Key
{
public:
    int x_, y_, w_, h_;       //键盘按钮的位置
    std::string text_;        //键盘按钮的字母

    /**
     * @brief Key构造函数
     * @param x     //键盘按钮的左上角位置x方向坐标
     * @param y     //键盘按钮的左上角位置y方向坐标
     * @param w     //键盘按钮的x方向宽度
     * @param h     //键盘按钮的y方向高度
     * @param text  //键盘按钮的值
     * @return None
     */
    Key(int x, int y, int w, int h, std::string text);

    /**
     * @brief Key析构函数
     * @return None
     */
    ~Key();

    /**
     * @brief 将键盘画到原图
     * @param img 原始图片
     * @param alpha 键盘按钮的透明度
     * @param fontScale 字体大小
     * @return None
     */
    void drawKey(cv::Mat& img, float alpha , double fontScale);

    /**
     * @brief 判断一个点是否在键盘按钮内
     * @param x 手指点的x坐标
     * @param y 手指点的y坐标
     * @return None
     */
    bool isOver(int x, int y);

};
#endif
