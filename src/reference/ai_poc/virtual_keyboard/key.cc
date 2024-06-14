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
#include "key.h"

Key::Key(int x, int y, int w, int h, std::string text)
: x_(x), y_(y), w_(w), h_(h), text_(text) 
{
}

Key::~Key()
{
}

void Key::drawKey(cv::Mat& img,  float alpha = 0.5, double fontScale = 2) 
{
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    int thickness = 2;
    cv::Scalar text_color = cv::Scalar(255, 255, 255, 255);
    cv::Scalar bg_color = cv::Scalar(255, 0, 0, 0);
    cv::Mat bg_rec = img(cv::Rect(x_, y_, w_, h_));
    cv::Mat white_rect(bg_rec.size(), bg_rec.type(), bg_color);
    cv::addWeighted(bg_rec, alpha, white_rect, 1 - alpha, 1.0, bg_rec);

    cv::Size text_size = cv::getTextSize(text_, fontFace, fontScale, thickness, nullptr);
    cv::Point text_pos(x_ + w_/2 - text_size.width/2, y_ + h_/2 + text_size.height/2);

    cv::putText(img, text_, text_pos, fontFace, fontScale, text_color, thickness);
}

bool Key::isOver(int x, int y) 
{
    if ((x_ + w_ > x) && (x > x_) && (y_ + h_ > y) && (y > y_)) {
        return true;
    }
    return false;
}
