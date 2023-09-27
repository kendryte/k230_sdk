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

#include "segment.h"

Seg::Seg(const char *kmodel_file, float conf_thres, float nms_thres, float mask_thres, const int debug_mode)
:conf_thres(conf_thres),nms_thres(nms_thres),mask_thres(mask_thres), AIBase(kmodel_file,"Seg", debug_mode)
{
    ai2d_out_tensor_ = this -> get_input_tensor(0);

    classes_count = classes.size();
    segChannels = 32;
    segWidth = input_shapes_[0][3]/4;
    segHeight = input_shapes_[0][2]/4;
    Num_box = (input_shapes_[0][3]/8) * (input_shapes_[0][2]/8) + (input_shapes_[0][3]/16) * (input_shapes_[0][2]/16) + (input_shapes_[0][3]/32) * (input_shapes_[0][2]/32);
}

Seg::Seg(const char *kmodel_file, float conf_thres, float nms_thres, float mask_thres, FrameCHWSize isp_shape, uintptr_t vaddr, uintptr_t paddr, const int debug_mode)
:conf_thres(conf_thres),nms_thres(nms_thres),mask_thres(mask_thres), AIBase(kmodel_file,"Seg", debug_mode)
{
    classes_count = classes.size();
    segChannels = 32;
    segWidth = input_shapes_[0][3]/4;
    segHeight = input_shapes_[0][2]/4;
    Num_box = (input_shapes_[0][3]/8) * (input_shapes_[0][2]/8) + (input_shapes_[0][3]/16) * (input_shapes_[0][2]/16) + (input_shapes_[0][3]/32) * (input_shapes_[0][2]/32);

    vaddr_ = vaddr;

    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    // int isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;

    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");

    // ai2d_out_tensor
    ai2d_out_tensor_ = get_input_tensor(0);
    // fixed padding resize param
    Utils::padding_resize(isp_shape_, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
}

Seg::~Seg()
{

}

void Seg::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
}

void Seg::pre_process()
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    size_t isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;
    auto buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(buf.data()), (void *)vaddr_, isp_size);
    hrt::sync(ai2d_in_tensor_, sync_op_t::sync_write_back, true).expect("sync write_back failed");
    ai2d_builder_->invoke(ai2d_in_tensor_, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void Seg::inference()
{
    this->run();
    this->get_output();
}

void Seg::post_process(FrameSize frame_size, vector<OutputSeg> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    output_0 = p_outputs_[0];
    output_1 = p_outputs_[1];

    int w, h, x, y;
	float r_w = input_shapes_[0][3] / (frame_size.width*1.0);
	float r_h = input_shapes_[0][2] / (frame_size.height*1.0);
	if (r_h > r_w) {
		w = input_shapes_[0][3];
		h = r_w * frame_size.height;
		x = 0;
		y = (input_shapes_[0][2] - h) / 2;
	}
	else {
		w = r_h * frame_size.width;
		h = input_shapes_[0][2];
		x = (input_shapes_[0][3] - w) / 2;
		y = 0;
	}

    int newh = h, neww = w, padh = y, padw = x;
	float ratio_h = (float)frame_size.height / newh;
	float ratio_w = (float)frame_size.width / neww;

    std::vector<int> classIds;//结果id数组
	std::vector<float> confidences;//结果每个id对应置信度数组
	std::vector<cv::Rect> boxes;//每个id矩形框
	std::vector<cv::Mat> picked_proposals;  //后续计算mask


	// 处理box
	int net_length = classes_count + 4 + segChannels;
	cv::Mat out1 = cv::Mat(net_length, Num_box, CV_32F, output_0);


	for (int i = 0; i < Num_box; i++) {
		//输出是1*net_length*Num_box;所以每个box的属性是每隔Num_box取一个值，共net_length个值
		cv::Mat scores = out1(Rect(i, 4, 1, classes_count)).clone();
		Point classIdPoint;
		double max_class_socre;
		minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
		max_class_socre = (float)max_class_socre;
		if (max_class_socre >= conf_thres) {
			cv::Mat temp_proto = out1(Rect(i, 4 + classes_count, 1, segChannels)).clone();
			picked_proposals.push_back(temp_proto.t());
			float x = (out1.at<float>(0, i) - padw) * ratio_w;  //cx
			float y = (out1.at<float>(1, i) - padh) * ratio_h;  //cy
			float w = out1.at<float>(2, i) * ratio_w;  //w
			float h = out1.at<float>(3, i) * ratio_h;  //h
			int left = MAX((x - 0.5 * w), 0);
			int top = MAX((y - 0.5 * h), 0);
			int width = (int)w;
			int height = (int)h;
			if (width <= 0 || height <= 0) { continue; }

			classIds.push_back(classIdPoint.y);
			confidences.push_back(max_class_socre);
			boxes.push_back(Rect(left, top, width, height));
		}

	}

	//执行非最大抑制以消除具有较低置信度的冗余重叠框（NMS）
	std::vector<int> nms_result;
	nms_boxes(boxes, confidences, conf_thres, nms_thres, nms_result);

	std::vector<cv::Mat> temp_mask_proposals;
	std::vector<OutputSeg> output;
	Rect holeImgRect(0, 0, frame_size.width, frame_size.height);
	for (int i = 0; i < nms_result.size(); ++i) {
		int idx = nms_result[i];
		OutputSeg result;
		result.id = classIds[idx];
		result.confidence = confidences[idx];
		result.box = boxes[idx]& holeImgRect;
		output.push_back(result);
		temp_mask_proposals.push_back(picked_proposals[idx]);
	}

	// 处理mask
	if(temp_mask_proposals.size() > 0)
	{	Mat maskProposals;
		for (int i = 0; i < temp_mask_proposals.size(); ++i)
		{
			maskProposals.push_back(temp_mask_proposals[i]);
		}

		Mat protos = Mat(segChannels, segWidth * segHeight, CV_32FC1, output_1);
		sync();
		Mat matmulRes = (maskProposals * protos).t();//n*32 32*25600 A*B是以数学运算中矩阵相乘的方式实现的，要求A的列数等于B的行数时
		Mat masks = matmulRes.reshape(output.size(), { segWidth,segHeight });//n*160*160

		std::vector<Mat> maskChannels;
		cv::split(masks, maskChannels);
		Rect roi(int((float)padw / input_shapes_[0][3] * segWidth), int((float)padh / input_shapes_[0][2] * segHeight), int(segWidth - padw / 2), int(segHeight - padh / 2));


		for (int i = 0; i < output.size(); ++i) {
			Mat dest, mask;
			cv::exp(-maskChannels[i], dest);//sigmoid
			dest = 1.0 / (1.0 + dest);//160*160
			dest = dest(roi);
			resize(dest, mask, cv::Size(frame_size.width, frame_size.height), INTER_NEAREST);
			//crop----截取box中的mask作为该box对应的mask
			Rect temp_rect = output[i].box;
			mask = mask(temp_rect) > mask_thres;
			output[i].boxMask = mask;
			output[i].label =classes[output[i].id];
		}
		results=output;
	}
}


void Seg::nms_boxes(vector<Rect> &boxes, vector<float> &confidences, float confThreshold, float nmsThreshold, vector<int> &indices)
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



float Seg::get_iou_value(Rect rect1, Rect rect2)
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