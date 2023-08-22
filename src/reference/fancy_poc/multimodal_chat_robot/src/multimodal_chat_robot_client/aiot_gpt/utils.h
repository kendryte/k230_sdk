#ifndef _UTILS
#define _UTILS

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

typedef struct BoxInfo
{
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
} BoxInfo;

typedef struct Framesize
{
    int width;
    int height;
} Framesize;

#define INTPUT_CHANNELS 3

cv::Mat read_image(const char *img_name);
cv::Mat pad_img_to_square(uint8_t *pimg, int valid_width, int valid_height);
cv::Mat letterbox(cv::Mat img, int width, int height);
cv::Mat od_get_scaled_img(cv::Mat img);
std::vector<uint8_t> hwc2chw(cv::Mat &img);

std::vector<BoxInfo> decode_infer(float *data, int net_size, int stride, int num_classes, Framesize frame_size, float anchors[][2], float threshold);
void nms(std::vector<BoxInfo> &input_boxes, float NMS_THRESH);
typedef struct
{
    float x;
    float y;
    float prob;
} point_t;

#if !NNCASE_SCOPEDTIMING
class ScopedTiming
{
public:
    ScopedTiming(std::string info = "ScopedTiming")
        : m_info(info)
    {
        m_start = std::chrono::steady_clock::now();
    }

    ~ScopedTiming()
    {
        m_stop = std::chrono::steady_clock::now();
        double elapsed_ms = std::chrono::duration<double, std::milli>(m_stop - m_start).count();
        std::cout << m_info << " took " << elapsed_ms << " ms" << std::endl;
    }

private:
    std::string m_info;
    std::chrono::steady_clock::time_point m_start;
    std::chrono::steady_clock::time_point m_stop;
};
#endif
#endif