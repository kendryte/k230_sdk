#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "posixtest.h"

#define EPSILON 0.000001

double sample_stddev(int count, ...)
{
    /* Compute the mean with args1. */
    double sum = 0;
    va_list args1;
    va_start(args1, count);
    va_list args2;
    va_copy(args2, args1); /* copy va_list object */
    for (int i = 0; i < count; ++i)
    {
        double num = va_arg(args1, double);
        sum += num;
    }
    va_end(args1);
    double mean = sum / count;

    /* Compute standard deviation with args2 and mean. */
    double sum_sq_diff = 0;
    for (int i = 0; i < count; ++i)
    {
        double num = va_arg(args2, double);
        sum_sq_diff += (num - mean) * (num - mean);
    }
    va_end(args2);
    return sqrt(sum_sq_diff / count);
}

int main(void)
{
    float fd = sample_stddev(4, 25.0, 27.3, 26.9, 25.7);
    if (fabs(fd - 0.920258) < EPSILON)
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }

    perror("va_copy error");
    return PTS_FAIL;
}