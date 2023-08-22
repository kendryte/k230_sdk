#include <iostream>
#include <chrono>
#include <fstream>
#include "mobilenetv2_depth.h"
#include "util.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;

Mobilenetv2Depth::Mobilenetv2Depth(const char *kmodel_file, size_t channel, size_t height, size_t width)
    : Model("Mobilenetv2Depth", kmodel_file), ai2d_input_c_(channel), ai2d_input_h_(height), ai2d_input_w_(width), threshold_(0.386), active_(false)
{
    // ai2d output tensor
    dims_t out_shape { 1, ai2d_input_c_, 128, 128 };
    ai2d_out_tensor_ = host_runtime_tensor::create(typecode_t::dt_uint16, out_shape).expect("cannot create input tensor");
}

Mobilenetv2Depth::~Mobilenetv2Depth()
{
}

void Mobilenetv2Depth::update_ai2d_config(int x, int y, int width, int height)
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    // ai2d config
    dims_t in_shape { 1, ai2d_input_c_, ai2d_input_h_, ai2d_input_w_ };
    dims_t out_shape { 1, ai2d_input_c_, 128, 128 };
    ai2d_datatype_t ai2d_dtype{ai2d_format::RAW16, ai2d_format::RAW16, typecode_t::dt_uint16, typecode_t::dt_uint16};

#if ENABLE_DEPTH_ROTATE
    // update coordinate
    x = ai2d_input_w_ - x - width;
    y = ai2d_input_h_ - y - height;
#endif
    ai2d_crop_param_t crop_param{true, x, y, width, height};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false};
    ai2d_builder_.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    ai2d_builder_->build_schedule();
}

void Mobilenetv2Depth::preprocess(uintptr_t vaddr, uintptr_t paddr)
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif
    size_t face_w = 128;
    size_t face_h = 128;

    // ai2d input tensor
    dims_t in_shape { 1, ai2d_input_c_, ai2d_input_h_, ai2d_input_w_ };
    auto ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint16, in_shape, { (gsl::byte *)vaddr, compute_size(in_shape) * sizeof(uint16_t) },
        false, hrt::pool_shared, paddr).expect("cannot create input tensor");
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

#if ENABLE_DEBUG
    dump_to_bin("depth_ai2d_input.bin", reinterpret_cast<void *>(vaddr), compute_size(in_shape) * sizeof(uint16_t));
#endif

    // run ai2d
    ai2d_builder_->invoke(ai2d_in_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
    auto src_buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    uint16_t *src = reinterpret_cast<uint16_t *>(src_buf.data());
#if ENABLE_DEBUG
    dump_to_bin("depth_ai2d_u16.bin", reinterpret_cast<void *>(src), src_buf.size());
#endif

    // uint16 to uint8
    uint64_t sum = 0;
    int count = 0;
    for (int cc = 0; cc < 1; cc++) {  //process only one channel
        int src_start_c = cc * face_w * face_h;
        for (int hh = (int)(face_h/4); hh < face_h-(int)(face_h/4); hh++) {
            int src_start_h = src_start_c + hh * face_w;
            for (int ww = (int)(face_w/4); ww < face_w-(int)(face_w/4); ww++) {
                uint16_t value = src[src_start_h + ww];
                if(value != 0){
                    sum += value;
                    count ++;
                }
            }
        }
    }
    uint16_t mean = sum / count;
    uint16_t min = mean - 112;
    uint16_t value_middle = 255 + min;

    auto kpu_in_tensor = input_tensor(0);
    auto dst_buf = kpu_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    uint8_t *dst = reinterpret_cast<uint8_t *>(dst_buf.data());

    for (int cc = 0; cc < 1; cc++) { //process only one channel
        int dst_start_c = cc * face_w * face_h;
        for (int hh = 0; hh < face_h; hh++) {
            int dst_start_h = dst_start_c + hh * face_w;
            for (int ww = 0; ww < face_w; ww++) {
                uint16_t value = src[hh * face_w + ww];
                int value_res =  value_middle - value;
                if(value == 0 || value_res < 0){
                    value_res = 0;
                }
                else if(value_res > 255){
                    value_res = 255;
                }
                dst[dst_start_h + ww] = value_res;
            }
        }
    }

    // copy 3 channels
    size_t size = face_w * face_h;
#if ENABLE_DEPTH_ROTATE
    // rotate 180
    uint8_t *p = dst + size * 2;
    for (size_t i = 0; i < face_h; i++)
    {
        size_t offset = (face_h - i - 1) * face_w;
        for (size_t j = 0; j < face_w; j++)
            p[offset + face_w - j - 1] = dst[i * face_w + j];
    }

    memcpy(dst, p, size);
    memcpy(dst + size, p, size);
#else
    memcpy(dst + size, dst, size);
    memcpy(dst + size * 2, dst, size);
#endif
    hrt::sync(kpu_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");
#if ENABLE_DEBUG
    dump_to_bin("depth_ai2d_u8.bin", reinterpret_cast<void *>(dst), dst_buf.size());
#endif
}

void Mobilenetv2Depth::postprocess()
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    auto tensor = output_tensor(0);
    auto buf = tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    float *output_data = reinterpret_cast<float *>(buf.data());
    float pred[2] = {0.0, 0.0};
    softmax(output_data, pred, sizeof(pred) / sizeof(pred[0]));
    active_ = pred[1] > threshold_ ? true : false;

#if ENABLE_DEBUG
    std::cout << model_name() << ": threshold_ = " << threshold_ << ", after softmax: pred[0] = " << pred[0] << ", pred[1] = " << pred[1] << std::endl;
#endif
}