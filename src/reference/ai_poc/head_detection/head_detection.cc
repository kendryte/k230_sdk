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

#include "head_detection.h"

HeadDetection::HeadDetection(const char *kmodel_file, float score_thres, float nms_thres, const int debug_mode)
:score_thres(score_thres), nms_thres(nms_thres), AIBase(kmodel_file,"HeadDetection", debug_mode)
{
    model_name_ = "HeadDetection";
    ai2d_out_tensor_ = this -> get_input_tensor(0);

    dimensions_det = classes.size() + 4;
    output_det = new float[output_shapes_[0][2] * dimensions_det];
}

HeadDetection::HeadDetection(const char *kmodel_file, float score_thres, float nms_thres, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr,const int debug_mode)
:score_thres(score_thres), nms_thres(nms_thres), AIBase(kmodel_file,"HeadDetection", debug_mode)
{
    model_name_ = "HeadDetection";

    dimensions_det = classes.size() + 4;
    output_det = new float[output_shapes_[0][2] * dimensions_det];

    vaddr_ = vaddr;

    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    // int isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;

    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");

    ai2d_out_tensor_ = this -> get_input_tensor(0);

    Utils::padding_resize(isp_shape, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
}

HeadDetection::~HeadDetection()
{
    delete[] output_det;
}

void HeadDetection::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_color_image("input_color.png", {input_shapes_[0][3],input_shapes_[0][2]},output);
}

void HeadDetection::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_, ai2d_out_tensor_).expect("error occurred in ai2d running");

    // auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    // unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());
    // Utils::dump_color_image("input_color_isp.png", {input_shapes_[0][3],input_shapes_[0][2]},output);
}

void HeadDetection::inference()
{
    this->run();
    this->get_output();
}

void HeadDetection::post_process(FrameSize frame_size, vector<Detection> &detections,bool pic_mode)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *ori_data = p_outputs_[0];

    float x_factor,y_factor;
    float dw,dh;
    if(pic_mode)
    {
        x_factor = float(frame_size.width) / input_shapes_[0][3];
        y_factor = float(frame_size.height) / input_shapes_[0][2];
        float ratio = (1/x_factor) < (1/y_factor) ? (1/x_factor) : (1/y_factor);
        int new_w = (int)(ratio * frame_size.width);
        int new_h = (int)(ratio * frame_size.height);
        dw = (float)(input_shapes_[0][3] - new_w) / 2 / ratio;   //dw for ori(not input_shapes)
        dh = (float)(input_shapes_[0][2] - new_h) / 2 / ratio;
        x_factor = float(frame_size.width + 2*dw) / input_shapes_[0][3];
        y_factor = float(frame_size.height + 2*dh) / input_shapes_[0][2];
    }
    else
    {
        x_factor = float(isp_shape_.width) / input_shapes_[0][3];
        y_factor = float(isp_shape_.height) / input_shapes_[0][2];
        float ratio = (1/x_factor) < (1/y_factor) ? (1/x_factor) : (1/y_factor);
        int new_w = (int)(ratio * isp_shape_.width);
        int new_h = (int)(ratio * isp_shape_.height);
        dw = (float)(input_shapes_[0][3] - new_w) / 2 / ratio;     //for isp shape
        dh = (float)(input_shapes_[0][2] - new_h) / 2 / ratio;
        x_factor = float(isp_shape_.width + 2*dw) / input_shapes_[0][3];
        y_factor = float(isp_shape_.height + 2*dh) / input_shapes_[0][2];
    }

    float *data = output_det;
    // ncw -> nwc
    for(int r = 0; r < output_shapes_[0][2]; r++)
    {
        for(int c = 0; c < dimensions_det; c++)
        {
            data[r*dimensions_det + c] = ori_data[c*output_shapes_[0][2] + r];
        }
    }


    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < output_shapes_[0][2]; ++i)
    {
        float *classes_scores = data+4;

        cv::Mat scores(1, classes.size(), CV_32FC1, classes_scores);
        cv::Point class_id;
        double maxClassScore;

        minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

        if (maxClassScore > score_thres)
        {
            confidences.push_back(maxClassScore);
            class_ids.push_back(class_id.x);

            float x = data[0];
            float y = data[1];
            float w = data[2];
            float h = data[3];

            int left = int((x - 0.5 * w) * x_factor) - dw;
            int top = int((y - 0.5 * h) * y_factor) - dh;

            int width = int(w * x_factor);
            int height = int(h * y_factor);

            boxes.push_back(cv::Rect(left, top, width, height));
        }

        data += dimensions_det;
    }

    std::vector<int> nms_result;
    nms_boxes(boxes, confidences, score_thres, nms_thres, nms_result);

    for (unsigned long i = 0; i < nms_result.size(); ++i)
    {
        int idx = nms_result[i];

        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(100, 255);
        result.color = cv::Scalar(dis(gen),
                                  dis(gen),
                                  dis(gen));

        result.className = classes[result.class_id];
        result.box = boxes[idx];
        detections.push_back(result);
    }

    delete[] data;
}


void HeadDetection::draw_result(cv::Mat& src_img,vector<Detection> &results, bool pic_mode)
{
    int src_w = src_img.cols;
    int src_h = src_img.rows;
    int max_src_size = std::max(src_w,src_h);

    int head_count = 0;
    for (int i = 0; i < results.size(); ++i)
    {
        auto& bbox = results[i].box;
        if(pic_mode)
        {
            if (debug_mode_ > 0)
            {
                cv::rectangle(src_img, cv::Rect(bbox.x, bbox.y , bbox.width, bbox.height), cv::Scalar(255, 255, 255), 2, 2, 0);
                std::string label_name = results[i].className;
                cv::putText(src_img, label_name , cv::Point(bbox.x,bbox.y), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, 8, 0);
                if(results[i].class_id==0)
                {    
                    head_count += 1;
                }
            }
            else
            {   
                if(results[i].class_id==0)
                {    
                    cv::rectangle(src_img, cv::Rect(bbox.x, bbox.y , bbox.width, bbox.height), cv::Scalar(255, 255, 255), 2, 2, 0);
                    head_count += 1;
                }
            }
        }
        else
        {		
            int x = std::max(0,int(float(bbox.x)/ isp_shape_.width * src_w));
            int y = std::max(0,int(float(bbox.y) / isp_shape_.height * src_h));
            int w = float(bbox.width) / isp_shape_.width * src_w;
            int h = float(bbox.height) / isp_shape_.height  * src_h;
            
            if (debug_mode_ > 0)
            {
                cv::rectangle(src_img, cv::Rect(x, y , w, h), cv::Scalar(255,0, 0, 255), 3, 2, 0);
                std::string label_name = results[i].className; 
                cv::putText(src_img,label_name,cv::Point(x,y),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255,255, 0, 255), 1, 8, 0);
                if(results[i].class_id==0)
                {    
                    head_count += 1;
                }
            }
            else
            {
                if(results[i].class_id==0)
                {    
                    cv::rectangle(src_img, cv::Rect(x, y , w, h), cv::Scalar(255,0, 0, 255), 3, 2, 0);
                    head_count += 1;
                }
            }           
        } 
    }

    if(pic_mode)
    {
        std::string str = "headcount : " + std::to_string(head_count);   
        cv::Size textSize = cv::getTextSize(str, cv::FONT_HERSHEY_COMPLEX, 0.5, 1, 0);
        cv::Rect textBox(0, 0, textSize.width, textSize.height+8);
        cv::rectangle(src_img, textBox, cv::Scalar(0, 0, 0), cv::FILLED);
        cv::putText(src_img, str , cv::Point(0, textSize.height - 1), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, 8, 0);
    }
    else
    {
        std::string str = "headcount : " + std::to_string(head_count); 
        cv::putText(src_img,str,cv::Point(10,50),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255,255, 0, 255), 1, 8, 0);
    }
}

void HeadDetection::nms_boxes(vector<Rect> &boxes, vector<float> &confidences, float confThreshold, float nmsThreshold, vector<int> &indices)
{	
	BBOX bbox;
	vector<BBOX> bboxes;
	int i, j;
	for (i = 0; i < boxes.size(); i++)
	{
		bbox.box = boxes[i];
		bbox.confidence = confidences[i];
		bbox.index = i;
		bboxes.push_back(bbox);
	}

	sort(bboxes.begin(), bboxes.end(), [](BBOX a, BBOX b) { return a.confidence < b.confidence; });

	int updated_size = bboxes.size();
	for (i = 0; i < updated_size; i++)
	{
		if (bboxes[i].confidence < confThreshold)
			continue;
		indices.push_back(bboxes[i].index);

		for (j = i + 1; j < updated_size;)
		{
			float iou = get_iou_value(bboxes[i].box, bboxes[j].box);

			if (iou > nmsThreshold)
			{
				bboxes.erase(bboxes.begin() + j);
				updated_size = bboxes.size();
			}
            else
            {
                j++;    
            }
		}
	}
}


float HeadDetection::get_iou_value(Rect rect1, Rect rect2)
{
	int xx1, yy1, xx2, yy2;
 
	xx1 = max(rect1.x, rect2.x);
	yy1 = max(rect1.y, rect2.y);
	xx2 = min(rect1.x + rect1.width - 1, rect2.x + rect2.width - 1);
	yy2 = min(rect1.y + rect1.height - 1, rect2.y + rect2.height - 1);
 
	int insection_width, insection_height;
	insection_width = max(0, xx2 - xx1 + 1);
	insection_height = max(0, yy2 - yy1 + 1);
 
	float insection_area, union_area, iou;
	insection_area = float(insection_width) * insection_height;
	union_area = float(rect1.width*rect1.height + rect2.width*rect2.height - insection_area);
	iou = insection_area / union_area;

	return iou;
}
