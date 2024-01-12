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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.f
 */


#include "tracker.h"

Tracker::Tracker(const char *kmodel_file, float thresh, const int debug_mode):thresh(thresh), AIBase(kmodel_file,"nanotracker-tracker", debug_mode)
{
    tracker_input_shapes[0] = {input_shapes_[0][0]*input_shapes_[0][1]*input_shapes_[0][2]*input_shapes_[0][3]};
    tracker_input_shapes[1] = {input_shapes_[1][0]*input_shapes_[1][1]*input_shapes_[1][2]*input_shapes_[1][3]};
}

Tracker::~Tracker()
{

}

void Tracker::pre_process(std::vector<float*> tracker_inputs)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    for ( int j = 0; j < 2; j++)
    {
        dims_t in_shape{input_shapes_[j][0], input_shapes_[j][1], input_shapes_[j][2], input_shapes_[j][3]};
        auto tensor = host_runtime_tensor::create(typecode_t::dt_float32, in_shape, hrt::pool_shared).expect("cannot create input tensor");
        auto mapped_buf = std::move(hrt::map(tensor, map_access_::map_write).unwrap());
        
        memcpy(reinterpret_cast<void *>(mapped_buf.buffer().data()), reinterpret_cast<void *>( tracker_inputs[j]), tracker_input_shapes[j]*4);
        auto ret = mapped_buf.unmap();
        ret = hrt::sync(tensor, sync_op_t::sync_write_back, true);
        if (!ret.is_ok())
        {
            std::cerr << "hrt::sync failed" << std::endl;
            std::abort();
        }

        this->set_input_tensor(j,tensor);
    }
}

void Tracker::inference()
{
    this->run();
    this->get_output();
}

void Tracker::post_process(int cols,int rows,std::vector<Tracker_box>& track_boxes)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    output_0 = p_outputs_[0];
    output_1 = p_outputs_[1];
    int box_x, box_y, box_h, box_w;
    float best_score;
    track_post_process(output_0, output_1, cols, rows, box_x, box_y, box_w, box_h, best_score);
    if (best_score > thresh)
    {
        Tracker_box track_box;
        track_box.x = box_x;
        track_box.y = box_y;
        track_box.w = box_w;
        track_box.h = box_h;
        track_box.score = best_score;
        track_boxes.push_back(track_box);
    }
}

void Tracker::draw_track(std::vector<Tracker_box> track_boxes,FrameSize sensor_size, FrameSize osd_size, cv::Mat& osd_frame)
{
    ScopedTiming st(model_name_ + " draw_track", debug_mode_);
    if (track_boxes.size())
    {
        int r_x1 = track_boxes[0].x;
        int r_y1 = track_boxes[0].y;
        int r_x2 = (r_x1+track_boxes[0].w);
        int r_y2 = (r_y1+track_boxes[0].h);

        int x1 =   r_x1*1.0 / sensor_size.width * osd_size.width;
        int y1 =    r_y1*1.0 / sensor_size.height  * osd_size.height;

        int w = (r_x2-r_x1)*1.0 / sensor_size.width * osd_size.width;
        int h = (r_y2-r_y1)*1.0 / sensor_size.height  * osd_size.height;

        cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255, 255,0, 0), 8, 2, 0); // ARGB
    }
}