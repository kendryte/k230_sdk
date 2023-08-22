#include <iostream>
#include <chrono>
#include <fstream>
#include "mobilenetv2_ir.h"
#include "util.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;

Mobilenetv2Ir::Mobilenetv2Ir(const char *kmodel_file, size_t channel, size_t height, size_t width)
    : Model("Mobilenetv2Ir", kmodel_file), ai2d_input_c_(channel), ai2d_input_h_(height), ai2d_input_w_(width), threshold_(0.51), active_(false)
{
    // ai2d input tensor
    dims_t in_shape { 1, ai2d_input_c_, ai2d_input_h_, ai2d_input_w_ };
    ai2d_in_tensor_ = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");

    // ai2d output tensor
    ai2d_out_tensor_ = input_tensor(0);
}

Mobilenetv2Ir::~Mobilenetv2Ir()
{
}

void Mobilenetv2Ir::update_ai2d_config(int x, int y, int width, int height)
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    // ai2d config
    dims_t in_shape { 1, ai2d_input_c_, ai2d_input_h_, ai2d_input_w_ };
    auto out_span = ai2d_out_tensor_.shape();
    dims_t out_shape { out_span.begin(), out_span.end() };

    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{true, x, y, width, height};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0, 0, 0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false};
    ai2d_builder_.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    ai2d_builder_->build_schedule();
}

void Mobilenetv2Ir::preprocess(uintptr_t vaddr, uintptr_t paddr)
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    // ai2d input tensor
    dims_t in_shape { 1, ai2d_input_c_, ai2d_input_h_, ai2d_input_w_ };
    auto ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, { (gsl::byte *)vaddr, compute_size(in_shape) },
        false, hrt::pool_shared, paddr).expect("cannot create input tensor");
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("sync write_back failed");

    // run ai2d
    ai2d_builder_->invoke(ai2d_in_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");

#if ENABLE_DEBUG
    {
        auto buf = ai2d_out_tensor_.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
        dump_to_bin("ir_ai2d.bin", reinterpret_cast<void *>(buf.data()), buf.size());
    }
#endif
}

void Mobilenetv2Ir::postprocess()
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    auto tensor = output_tensor(0);
    auto buf = tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    float *output_data = reinterpret_cast<float *>(buf.data());
    float pred[2] = { 0.0, 0.0 };
    softmax(output_data, pred, sizeof(pred) / sizeof(pred[0]));
    active_ = pred[1] > threshold_ ? true : false;

#if ENABLE_DEBUG
    std::cout << model_name() << ": threshold_ = " << threshold_ << ", after softmax: pred[0] = " << pred[0] << ", pred[1] = " << pred[1] << std::endl;
#endif
}