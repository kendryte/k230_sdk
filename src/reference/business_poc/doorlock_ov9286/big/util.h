#ifndef _UTIL_H
#define _UTIL_H
#include <stdint.h>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#define mask_thresh_ 0.75
#define ENABLE_RVV 1
#define ENABLE_DEPTH_ROTATE 1
#define ENABLE_DEBUG 0
#define ENABLE_PROFILING 0
#define PARAM_PATH          "/bin/H1280W720_conf.bin"
#define REF_PATH            "/bin/H1280W720_ref.bin"
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

typedef struct FaceMaskInfo
{
    std::string label;   
    float score;
    bool is_success;
} FaceMaskInfo;

typedef struct feature
{
    char name[64];   
    float feature[192];
} feature;

typedef struct feature_db
{
    uint32_t count;
    feature feature_db_data[0];
} feature_db_t;

typedef struct
{
    int x1;
    int y1;
    int x2;
    int y2;
} face_coordinate;

typedef enum
{
    MSG_CMD_SIGNUP,
    MSG_CMD_SIGNUP_RESULT,
    MSG_CMD_IMPORT,
    MSG_CMD_IMPORT_RESULT,
    MSG_CMD_DELETE,
    MSG_CMD_DELETE_RESULT,
    MSG_CMD_FEATURE_SAVE,
    MSG_CMD_ERROR,
}ipc_msg_cmd_t;

typedef struct
{
	float points[10];
} landmarks_t;

void read_binary_file(const char *file_name, char *buffer);
template <class T>
std::vector<T> read_binary_file(const char *file_name)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    std::vector<T> vec(len / sizeof(T), 0);
    ifs.seekg(0, ifs.beg);
    ifs.read(reinterpret_cast<char *>(vec.data()), len);
    ifs.close();
    return std::move(vec);
}
void dump(const std::string &info, volatile float *p, size_t size);
void dump_to_bin(const char *bin_name, void *buffer, size_t size);
void softmax(float *x, float *dx, uint32_t len);
void image_umeyama(float* src, float* dst);
uint32_t l2normalize(const float* x, float* dx, int len);
float calCosinDistance(float* a, float* b, int len);
size_t calulate_score(uint32_t count,std::vector<float> &feature, float* score);
size_t l2normalize_feature_db(feature_db_t *feature_data,uint32_t count);
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
