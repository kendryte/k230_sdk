#include <iostream>
#include <chrono>
#include <fstream>
#include "mobile_face.h"
#include "util.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;

MobileFace::MobileFace(const char *kmodel_file, size_t channel, size_t height, size_t width)
    : Model("MobileFace", kmodel_file), ai2d_input_c_(channel), ai2d_input_h_(height), ai2d_input_w_(width)
{
    ai2d_out_tensor_ = input_tensor(0);
    result_.resize(compute_size(output_shape(0)), 0);
}

MobileFace::~MobileFace()
{
}

void MobileFace::update_ai2d_config(landmarks_t landmark)
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    float matrix_src[5][2];
    std::vector<float> M(6, 0);
    for (uint32_t i = 0; i < 5; i++)
    {
        matrix_src[i][0] = (int)(landmark.points[2 * i + 0]);
        matrix_src[i][1] = (int)(landmark.points[2 * i + 1]);
    }
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
        dump_to_bin("mbface_ai2d.bin", reinterpret_cast<void *>(buf.data()), buf.size());
    }
#endif
}

void MobileFace::postprocess()
{
#if ENABLE_PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    auto tensor = output_tensor(0);
    auto buf = tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
    float *output_data = reinterpret_cast<float *>(buf.data());
    memcpy(result_.data(), output_data, result_.size() * sizeof(float));
}