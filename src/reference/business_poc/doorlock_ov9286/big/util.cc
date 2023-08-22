#include <math.h>
#include <iostream>
#include <fstream>
#include "util.h"

#if ENABLE_RVV
#include "rvv_math.h"
#endif

static std::vector<float> dataset_l2norm;
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

void dump_to_bin(const char *bin_name, void *buffer, size_t size)
{
	std::fstream fp(bin_name, std::ios::out | std::ios::binary);
	fp.write(reinterpret_cast<char *>(buffer), size);
	fp.close();
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

static void svd22(const float a[4], float u[4], float s[2], float v[4])
{
	s[0] = (sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)) + sqrtf(powf(a[0] + a[3], 2) + powf(a[1] - a[2], 2))) / 2;
	s[1] = fabsf(s[0] - sqrtf(powf(a[0] - a[3], 2) + powf(a[1] + a[2], 2)));
	v[2] = (s[0] > s[1]) ? sinf((atan2f(2 * (a[0] * a[1] + a[2] * a[3]), a[0] * a[0] - a[1] * a[1] + a[2] * a[2] - a[3] * a[3])) / 2) : 0;
	v[0] = sqrtf(1 - v[2] * v[2]);
	v[1] = -v[2];
	v[3] = v[0];
	u[0] = (s[0] != 0) ? -(a[0] * v[0] + a[1] * v[2]) / s[0] : 1;
	u[2] = (s[0] != 0) ? -(a[2] * v[0] + a[3] * v[2]) / s[0] : 0;
	u[1] = (s[1] != 0) ? (a[0] * v[1] + a[1] * v[3]) / s[1] : -u[2];
	u[3] = (s[1] != 0) ? (a[2] * v[1] + a[3] * v[3]) / s[1] : u[0];
	v[0] = -v[0];
	v[2] = -v[2];
}

void image_umeyama(float* src, float* dst)
{
	float umeyama_args[] =
	{
		43.7652, 59.0814,
		84.0363, 58.8587,
		64.0288, 81.9846,
		47.4849, 105.560,
		80.8341, 105.376
	};

#define SRC_NUM 5
#define SRC_DIM 2
	int i, j, k;
	float src_mean[SRC_DIM] = { 0.0 };
	float dst_mean[SRC_DIM] = { 0.0 };
	for (i = 0; i < SRC_NUM * 2; i += 2)
	{
		src_mean[0] += src[i];
		src_mean[1] += src[i + 1];
		dst_mean[0] += umeyama_args[i];
		dst_mean[1] += umeyama_args[i + 1];
	}
	src_mean[0] /= SRC_NUM;
	src_mean[1] /= SRC_NUM;
	dst_mean[0] /= SRC_NUM;
	dst_mean[1] /= SRC_NUM;

	float src_demean[SRC_NUM][2] = { 0.0 };
	float dst_demean[SRC_NUM][2] = { 0.0 };

	for (i = 0; i < SRC_NUM; i++)
	{
		src_demean[i][0] = src[2 * i] - src_mean[0];
		src_demean[i][1] = src[2 * i + 1] - src_mean[1];
		dst_demean[i][0] = umeyama_args[2 * i] - dst_mean[0];
		dst_demean[i][1] = umeyama_args[2 * i + 1] - dst_mean[1];
	}

	float A[SRC_DIM][SRC_DIM] = { 0.0 };
	for (i = 0; i < SRC_DIM; i++)
	{
		for (k = 0; k < SRC_DIM; k++)
		{
			for (j = 0; j < SRC_NUM; j++)
			{
				A[i][k] += dst_demean[j][i] * src_demean[j][k];
			}
			A[i][k] /= SRC_NUM;
		}
	}

	float(*T)[SRC_DIM + 1] = (float(*)[SRC_DIM + 1]) dst;
	T[0][0] = 1;
	T[0][1] = 0;
	T[0][2] = 0;
	T[1][0] = 0;
	T[1][1] = 1;
	T[1][2] = 0;
	T[2][0] = 0;
	T[2][1] = 0;
	T[2][2] = 1;

	float U[SRC_DIM][SRC_DIM] = { 0 };
	float S[SRC_DIM] = { 0 };
	float V[SRC_DIM][SRC_DIM] = { 0 };
	svd22(&A[0][0], &U[0][0], S, &V[0][0]);

	T[0][0] = U[0][0] * V[0][0] + U[0][1] * V[1][0];
	T[0][1] = U[0][0] * V[0][1] + U[0][1] * V[1][1];
	T[1][0] = U[1][0] * V[0][0] + U[1][1] * V[1][0];
	T[1][1] = U[1][0] * V[0][1] + U[1][1] * V[1][1];

	float scale = 1.0;
	float src_demean_mean[SRC_DIM] = { 0.0 };
	float src_demean_var[SRC_DIM] = { 0.0 };
	for (i = 0; i < SRC_NUM; i++)
	{
		src_demean_mean[0] += src_demean[i][0];
		src_demean_mean[1] += src_demean[i][1];
	}
	src_demean_mean[0] /= SRC_NUM;
	src_demean_mean[1] /= SRC_NUM;

	for (i = 0; i < SRC_NUM; i++)
	{
		src_demean_var[0] += (src_demean_mean[0] - src_demean[i][0]) * (src_demean_mean[0] - src_demean[i][0]);
		src_demean_var[1] += (src_demean_mean[1] - src_demean[i][1]) * (src_demean_mean[1] - src_demean[i][1]);
	}
	src_demean_var[0] /= (SRC_NUM);
	src_demean_var[1] /= (SRC_NUM);
	scale = 1.0 / (src_demean_var[0] + src_demean_var[1]) * (S[0] + S[1]);
	T[0][2] = dst_mean[0] - scale * (T[0][0] * src_mean[0] + T[0][1] * src_mean[1]);
	T[1][2] = dst_mean[1] - scale * (T[1][0] * src_mean[0] + T[1][1] * src_mean[1]);
	T[0][0] *= scale;
	T[0][1] *= scale;
	T[1][0] *= scale;
	T[1][1] *= scale;
}

uint32_t l2normalize(float* x, float* dx, int len)
{
#if ENABLE_RVV
	normalize_vec(len, x, dx);
#else
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
#endif
	return 0;
}

float calCosinDistance(float* a, float* b, int len)
{
	float coorFeature = 0;

#if ENABLE_RVV
	coorFeature = cosin_distance_vec(len, a, b);
#else
	// calculate the sum square
	for (int i = 0; i < featureLen; i++)
	{
		float featureVal0 = *(a + i);
		float featureVal1 = *(b + i);
		coorFeature += featureVal0 * featureVal1;
	}
#endif

	// cosin distance
	return 0.5 + 0.5 * coorFeature;
}


size_t l2normalize_feature_db(feature_db_t *feature_data,uint32_t count)
{
	int size = 192;
	dataset_l2norm.clear();
	if (dataset_l2norm.empty())
	{
		dataset_l2norm.resize(count*size);
		for (size_t i = 0; i < count; i++)
		{
			l2normalize((float*)(feature_data->feature_db_data[i].feature), dataset_l2norm.data() + i * size, size);
		}
	}
}


size_t calulate_score(uint32_t count,std::vector<float> &feature, float* score)
{
	size_t idx = 0;
	float score_max = 0.0;
	size_t size = feature.size();
	
	std::vector<float> test_l2norm(size, 0);
	l2normalize(feature.data(), test_l2norm.data(), size);
	if (!dataset_l2norm.empty())
	{
		for (size_t i = 0; i < count; i++)
		{
			auto s = calCosinDistance(dataset_l2norm.data() + i * size, test_l2norm.data(), size);
			if (s > score_max)
			{
				score_max = s;
				idx = i;
			}
		}
	}
	*score = score_max;
	return idx;
}
