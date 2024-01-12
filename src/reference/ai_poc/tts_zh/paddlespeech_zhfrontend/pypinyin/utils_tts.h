#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>



int getBinSize(std::string path);


void readBin_float(std::string path, float *buf, int size);

void readBin_int(std::string path, int* buf, int size);

void writeBin_float(std::string path, float* buf,int size);

void writeBin_int(std::string path, int* buf,int size);


float getMold(float* vec ,int n);
   

float getMold_int(int* vec ,int n);

float getSimilarity(float* lhs, float* rhs,int n);

float getSimilarity_int(int* lhs, int* rhs,int n);

int GetSysMemInfo();

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
        elapsed_ms = std::chrono::duration<double, std::milli>(m_stop - m_start).count();
        std::cout << m_info << " took " << elapsed_ms << " ms" << std::endl;
    }

    double elapsed_ms;

private:
    std::string m_info;
    std::chrono::steady_clock::time_point m_start;
    std::chrono::steady_clock::time_point m_stop;
    
};