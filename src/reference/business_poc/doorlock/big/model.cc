#include <iostream>
#include <chrono>
#include <fstream>
#include "model.h"
#include "util.h"

Model::Model(const char *model_name, const char *kmodel_file): model_name_(model_name)
{
    // load kmodel
    kmodel_ = read_binary_file<unsigned char>(kmodel_file);
    interp_.load_model({ (const gsl::byte *)kmodel_.data(), kmodel_.size() }).expect("cannot load kmodel.");

    // create kpu input tensors
    for (size_t i = 0; i < interp_.inputs_size(); i++)
    {
        auto desc = interp_.input_desc(i);
        auto shape = interp_.input_shape(i);
        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
        interp_.input_tensor(i, tensor).expect("cannot set input tensor");
    }

    // create kpu output tensors
    // for (size_t i = 0; i < interp_.outputs_size(); i++)
    // {
    //     auto desc = interp_.output_desc(i);
    //     auto shape = interp_.output_shape(i);
    //     auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create output tensor");
    //     interp_.output_tensor(i, tensor).expect("cannot set output tensor");
    // }
}

Model::~Model()
{

}

void Model::run(uintptr_t vaddr, uintptr_t paddr)
{
    preprocess(vaddr, paddr);
    kpu_run();
    postprocess();
}

std::string Model::model_name() const
{
    return model_name_;
}

void Model::kpu_run()
{
#if PROFILING
    ScopedTiming st(model_name() + " " + __FUNCTION__);
#endif

    interp_.run().expect("error occurred in running model");
}

runtime_tensor Model::input_tensor(size_t idx)
{
    return interp_.input_tensor(idx).expect("cannot get input tensor");
}

void Model::input_tensor(size_t idx, runtime_tensor &tensor)
{
    interp_.input_tensor(idx, tensor).expect("cannot set input tensor");
}

runtime_tensor Model::output_tensor(size_t idx)
{
    return interp_.output_tensor(idx).expect("cannot get output tensor");
}

dims_t Model::output_shape(size_t idx)
{
    return interp_.output_shape(idx);
}
