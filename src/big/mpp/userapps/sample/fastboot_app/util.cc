#include <math.h>
#include <iostream>
#include <fstream>
#include "util.h"

void read_binary_file(const char *file_name, char *buffer, size_t size)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
	size_t len = ifs.tellg();
	if (size != 0)
	{
		len = size;
	}

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
