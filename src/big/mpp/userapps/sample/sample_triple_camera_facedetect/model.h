#ifndef _MODEL_H
#define _MODEL_H

#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/functional/ai2d/ai2d_builder.h>
#include "util.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;
using namespace nncase::F::k230;

class Model
{
public:
    Model(const char *model_name, const char *kmodel_file);
    ~Model();
    void run(uintptr_t vaddr, uintptr_t paddr);
    std::string model_name() const;

protected:
    virtual void preprocess(uintptr_t vaddr, uintptr_t paddr) = 0;
    void kpu_run();
    virtual void postprocess() = 0;
    runtime_tensor input_tensor(size_t idx);
    void input_tensor(size_t idx, runtime_tensor &tensor);
    runtime_tensor output_tensor(size_t idx);
    dims_t input_shape(size_t idx);
    dims_t output_shape(size_t idx);
protected:
    std::unique_ptr<ai2d_builder> ai2d_builder_;
    runtime_tensor ai2d_in_tensor_;
    runtime_tensor ai2d_out_tensor_;

private:
    interpreter interp_;
    std::string model_name_;
    std::vector<uint8_t> kmodel_;
};
#endif
