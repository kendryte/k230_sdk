#ifndef _UTIL_H
#define _UTIL_H
#include <stdint.h>
#include <chrono>

#define PROFILING 0

#define FL 192
typedef struct
{
	int index;
	float* probs;
} sortable_obj_t;

typedef struct
{
	float x;
	float y;
	float w;
	float h;
} box_t;

typedef struct
{
	float points[10];
} landmarks_t;

void read_binary_file(const char *file_name, char *buffer);
void dump(const std::string &info, volatile float *p, size_t size);
void softmax(float *x, float *dx, uint32_t len);
uint32_t l2normalize(const float* x, float* dx, int len);
float calCosinDistance(float* faceFeature0P, float* faceFeature1P, int featureLen);
uint32_t calulate_score(const float* features, const float* saved_features, uint32_t saved_len, float* score);

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
