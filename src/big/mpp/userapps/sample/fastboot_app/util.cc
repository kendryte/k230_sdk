#include <math.h>
#include <iostream>
#include <fstream>
#include "util.h"

void read_binary_file(const char *file_name, char *buffer)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    ifs.seekg(0, ifs.beg);
    ifs.read(buffer, len);
    ifs.close();
}

void dump(const std::string &info, volatile float *p, size_t size)
{
    std::cout << info << " dump: p = " << std::hex << (void *)p << std::dec << ", size = " << size << std::endl;
    volatile unsigned int *q = reinterpret_cast<volatile unsigned int *>(p);
    for (size_t i = 0; i < size; i++)
    {
        if ((i != 0) && (i % 4 == 0))
        {
            std::cout << std::endl;
        }

        std::cout << std::hex << q[i] << " ";
    }
    std::cout << std::dec << std::endl;
}

void softmax(float* x, float* dx, uint32_t len)
{
    float max_value = x[0];
    for (uint32_t i = 0; i < len; i++)
    {
        if (max_value < x[i])
        {
            max_value = x[i];
        }
    }
    for (uint32_t i = 0; i < len; i++)
    {
        x[i] -= max_value;
        x[i] = expf(x[i]);
    }
    float sum_value = 0.0f;
    for (uint32_t i = 0; i < len; i++)
    {
        sum_value += x[i];
    }
    for (uint32_t i = 0; i < len; i++)
    {
        dx[i] = x[i] / sum_value;
    }
}


uint32_t l2normalize(const float* x, float* dx, int len)
{
	int f;
	float sum = 0;
	for (f = 0; f < len; ++f)
	{
		sum += x[f] * x[f];
	}
	sum = sqrtf(sum);
	for (f = 0; f < len; ++f)
	{
		dx[f] = x[f] / sum;
	}
	return 0;
}

float calCosinDistance(float* faceFeature0P, float* faceFeature1P, int featureLen)
{
	float coorFeature = 0;
	// calculate the sum square
	for (int fIdx = 0; fIdx < featureLen; fIdx++)
	{
		float featureVal0 = *(faceFeature0P + fIdx);
		float featureVal1 = *(faceFeature1P + fIdx);
		coorFeature += featureVal0 * featureVal1;
	}
	// cosin distance
	return (0.5 + 0.5 * coorFeature) * 100;
}

uint32_t calulate_score(const float* features, const float* saved_features, uint32_t saved_len, float* score)
{
	int i;
	int v_id = -1;
	float v_score;
	float v_score_max = 0.0;
	float basef[FL], testf[FL];
	l2normalize(features, testf, FL);
	for (i = 0; i < saved_len; i++)
	{
		l2normalize(saved_features + i * FL, basef, FL);
		v_score = calCosinDistance(testf, basef, FL);
		if (v_score > v_score_max)
		{
			v_score_max = v_score;
			v_id = i;
		}
	}
	*score = v_score_max;
	return v_id;
}