#ifndef _MOBILE_RETINAFACE_H
#define _MOBILE_RETINAFACE_H
#include "model.h"
#include "util.h"

#define ENABLE_RVV 1
#if ENABLE_RVV
#include "k230_math.h"
#include "rvv_math.h"
#endif

class MobileRetinaface : public Model
{
public:
    MobileRetinaface(const char *kmodel_file, size_t channel, size_t height, size_t width);
    ~MobileRetinaface();
    std::vector<int> get_result() const
    {
        return face_boxes_;
    }

protected:
    void preprocess(uintptr_t vaddr, uintptr_t paddr);
    void postprocess();

private:
    void decode(box_t *pred_box, size_t *box_num, landmarks_t *pred_landmarks, size_t *landmark_num);
    float overlap(float x1, float w1, float x2, float w2);
    float box_intersection(box_t a, box_t b);
    float box_union(box_t a, box_t b);
    float box_iou(box_t a, box_t b);
#if !ENABLE_RVV
    void deal_conf(float *conf, float *s_probs, sortable_obj_t *s, int size, int *obj_cnt);
    void deal_loc(float *loc, float *boxes, int size, int *obj_cnt);
    void deal_landms(float *landms, float *landmarks, int size, int *obj_cnt);
    box_t get_box(float *boxes, int obj_index);
    landmarks_t get_landmark(float *landmarks, int obj_index);
#else
    void deal_conf_opt(float *conf, float *s_probs, int *s, int size, int *obj_cnt, int *real_count, float RETINAFACE_OBJ_THRESH, float *tmp);
    void deal_loc_opt(float *loc, float *boxes, int size, int *obj_cnt, int *s, int *real_count);
    void deal_landms_opt(float *landms, float *landmarks, int size, int *obj_cnt, int *s, int *real_count);
    box_t get_box_opt(float *boxes, int obj_index, int index_anchors);
    landmarks_t get_landmark_opt(float *landmarks, int obj_index, int index_anchors);
#endif

private:
    size_t ai2d_input_c_;
    size_t ai2d_input_h_;
    size_t ai2d_input_w_;
    uintptr_t vaddr_;
    uintptr_t paddr_;
    std::vector<int> face_boxes_;
};

#endif
