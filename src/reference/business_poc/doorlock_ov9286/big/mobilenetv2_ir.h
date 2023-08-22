#ifndef _MOBILENETV2_IR_H
#define _MOBILENETV2_IR_H
#include "model.h"
#include "util.h"

class Mobilenetv2Ir : public Model
{
public:
    Mobilenetv2Ir(const char *kmodel_file, size_t channel, size_t height, size_t width);
    ~Mobilenetv2Ir();
    void update_ai2d_config(int x, int y, int width, int height);
    bool get_result() const
    {
        return active_;
    }

protected:
    void preprocess(uintptr_t vaddr, uintptr_t paddr);
    void postprocess();

private:
    size_t ai2d_input_c_;
    size_t ai2d_input_h_;
    size_t ai2d_input_w_;
    float threshold_;
    bool active_;
};

#endif
