#ifndef _MOBILE_FACE_H
#define _MOBILE_FACE_H
#include "model.h"
#include "util.h"

class MobileFace : public Model
{
public:
    MobileFace(const char *kmodel_file, size_t channel, size_t height, size_t width);
    ~MobileFace();
    void update_ai2d_config(landmarks_t landmark);
    std::vector<float> get_result() const
    {
        return result_;
    }

protected:
    void preprocess(uintptr_t vaddr, uintptr_t paddr);
    void postprocess();

private:
    size_t ai2d_input_c_;
    size_t ai2d_input_h_;
    size_t ai2d_input_w_;
    std::vector<float> result_;
};

#endif
