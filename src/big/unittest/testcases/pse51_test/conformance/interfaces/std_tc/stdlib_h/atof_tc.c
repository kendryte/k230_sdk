#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "posixtest.h"

#define uassert_float_equal(a, b) assert((a) == (b))
#define uassert_float_not_equal(a, b) assert((a) != (b))

struct atof_data
{
    char string[32];
    float ret_num;
};

struct atof_data test_data[] =
    {
        /* positive float */
        {"0", 0},
        {"1", 1.0f},
        {"1.123", 1.123f},
        {"123", 123.0f},
        {"3.402823", 3.402823f},
        {"123.402823", 123.402823f},
        {"123456.402823", 123456.402823f},

        /* negtive float */
        {"-1", -1.0f},
        {"-1.123", -1.123f},
        {"-123", -123.0f},
        {"-3.402823", -3.402823f},
        {"-123.402823", -123.402823f},
        {"-123456.402823", -123456.402823f},

        /* letters and numbers */
        {"12.5a45", 12.5f},
        {"-12.5a45.5", -12.5f},
        {"12.5/45", 12.5f},
        {"-12.5/45", -12.5f},

        /* cannot be resolved */
        {"", 0},
        {" ", 0},
        {"abc12.2", 0},
        {" abc12.2", 0},
        {NULL, -1}};

int atof_entry(void)
{
    int i = 0;
    float res = 0;
    for (i = 0; i < sizeof(test_data) / sizeof(test_data[0]); i++)
    {
        res = atof(test_data[i].string);
        printf("%f\n", res);
        uassert_float_equal(res, test_data[i].ret_num);
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main()
{
    atof_entry();
    return 0;
}
