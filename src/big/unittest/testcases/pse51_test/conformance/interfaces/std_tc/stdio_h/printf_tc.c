#include <stdio.h>
#include "posixtest.h"

static int printf_entry(void)
{
    int res = 0;
    res = printf("printf test:%s-%d-%c %f 0x%x", "2021", 8, '1', 3.14, 0xff);
    if(res <= 0)
    {
        printf("Test failed\n");
        return PTS_FAIL;
    }
    printf("{Test PASSED}\n");

    return PTS_PASS;
}

int main(void)
{
    printf_entry();
    return 0;
}
