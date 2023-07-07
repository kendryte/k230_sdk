#include <iostream>
#include <chrono>
#include <fstream>
#include "mobile_face.h"
#include "util.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;
static int count = 0; 
MobileFace::MobileFace(const char *kmodel_file, size_t channel, size_t height, size_t width)
    : Model("MobileFace", kmodel_file), ai2d_input_c_(channel), ai2d_input_h_(height), ai2d_input_w_(width)
{
    // ai2d output tensor
    ai2d_out_tensor_ = input_tensor(0);

    result_.resize(compute_size(output_shape(0)), 0);
}

MobileFace::~MobileFace()
{
}

void MobileFace::update_ai2d_config(landmarks_t landmark)
{
#if PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    int long_side = ai2d_input_h_ > ai2d_input_w_ ? ai2d_input_h_ : ai2d_input_w_;
    float matrix_src[5][2];
    std::vector<float> M(6, 0);
    for (uint32_t i = 0; i < 5; i++)
    {
        matrix_src[i][0] = (int)(landmark.points[2 * i + 0] * long_side - 280);
        matrix_src[i][1] = (int)(landmark.points[2 * i + 1] * long_side);
    }
    // for(int t = 0;t<5;t++)
    // {
    //     printf("%f-%f\n",landmark.points[2*t + 0],landmark.points[2*t + 1]);
    // }
    image_umeyama(&matrix_src[0][0], M.data());
    // ai2d config

    dims_t in_shape { 1, ai2d_input_c_, ai2d_input_h_, ai2d_input_w_ };
    auto out_span = ai2d_out_tensor_.shape();
    dims_t out_shape { out_span.begin(), out_span.end() };

    ai2d_datatype_t ai2d_dtype { ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8 };
    ai2d_crop_param_t crop_param { false, 0, 0, 0, 0 };
    ai2d_shift_param_t shift_param { false, 0 };
    ai2d_pad_param_t pad_param { false, { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, ai2d_pad_mode::constant, { 0, 0, 0 } };
    ai2d_resize_param_t resize_param { false, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel };
    ai2d_affine_param_t affine_param { true, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, M };
    ai2d_builder_.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    ai2d_builder_->build_schedule();
}

void MobileFace::preprocess(uintptr_t vaddr, uintptr_t paddr)
{
#if PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif
    char file_name1[100];
    char file_name[100];
    // ai2d input tensor
    dims_t in_shape { 1, ai2d_input_c_, ai2d_input_h_, ai2d_input_w_ };
    auto ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, compute_size(in_shape) },
        false, hrt::pool_shared, paddr).expect("cannot create input tensor");
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("sync write_back failed");

    // run ai2d
    ai2d_builder_->invoke(ai2d_in_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
    // {
    //     auto vaddr_out_buf = ai2d_in_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    //     unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

    //     cv::Mat image_r = cv::Mat(720, 1280, CV_8UC1, output);
    //     cv::Mat image_g = cv::Mat(720, 1280, CV_8UC1, output+1280*720);
    //     cv::Mat image_b = cv::Mat(720, 1280, CV_8UC1, output+2*1280*720);

    //     std::vector<cv::Mat> color_vec(3);
    //     color_vec.clear();
    //     color_vec.push_back(image_b);
    //     color_vec.push_back(image_g);
    //     color_vec.push_back(image_r);

    //     cv::Mat color_img;
    //     cv::merge(color_vec, color_img);
    //     sprintf(file_name,"/sharefs/1_%d.jpg",count++);
    //     cv::imwrite(file_name, color_img);
    // }
    // {
    //     auto vaddr_out_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    //     unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

    //     cv::Mat image_r = cv::Mat(128, 128, CV_8UC1, output);
    //     cv::Mat image_g = cv::Mat(128, 128, CV_8UC1, output+128*128);
    //     cv::Mat image_b = cv::Mat(128, 128, CV_8UC1, output+2*128*128);

    //     std::vector<cv::Mat> color_vec(3);
    //     color_vec.clear();
    //     color_vec.push_back(image_b);
    //     color_vec.push_back(image_g);
    //     color_vec.push_back(image_r);

    //     cv::Mat color_img;
    //     cv::merge(color_vec, color_img);
    //     sprintf(file_name1,"/sharefs/2_%d.jpg",count++);
    //     if(count % 3 == 0)
    //         cv::imwrite(file_name1, color_img);
    // }
}

void MobileFace::postprocess()
{
#if PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    auto tensor = output_tensor(0);
    auto buf = tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    float *output_data = reinterpret_cast<float *>(buf.data());
    memcpy(result_.data(), output_data, result_.size() * sizeof(float));
}