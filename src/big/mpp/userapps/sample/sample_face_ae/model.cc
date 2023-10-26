#include <iostream>
#include <chrono>
#include <fstream>
#include "model.h"
#include "util.h"

Model::Model(const char *model_name, const char *kmodel_file): model_name_(model_name)
{
    // load kmodel
    std::ifstream ifs(kmodel_file, std::ios::binary);
    interp_.load_model(ifs).expect("load_model failed");

    // create kpu input tensors
    for (size_t i = 0; i < interp_.inputs_size(); i++)
    {
        auto desc = interp_.input_desc(i);
        auto shape = interp_.input_shape(i);
        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared).expect("cannot create input tensor");
        interp_.input_tensor(i, tensor).expect("cannot set input tensor");
    }
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
#if ENABLE_PROFILING
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

dims_t Model::input_shape(size_t idx)
{
    return interp_.input_shape(idx);
}

dims_t Model::output_shape(size_t idx)
{
    return interp_.output_shape(idx);
}
