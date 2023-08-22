#ifndef _MOBILENETV2_DEPTH_H
#define _MOBILENETV2_DEPTH_H
#include "model.h"
#include "util.h"

class Mobilenetv2Depth : public Model
{
public:
    Mobilenetv2Depth(const char *kmodel_file, size_t channel, size_t height, size_t width);
    ~Mobilenetv2Depth();
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
