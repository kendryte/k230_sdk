#include <stdio.h>
#include "posixtest.h"

static int putchar_entry(void)
{
    char data[] = "putchar testcase\n";
    int i = 0;
    int res = 0;
    for (; i < sizeof(data); i++)
    {
        res = putchar(data[i]);
        if(res != data[i])
        {
            printf("Test failed\n");
            return PTS_FAIL;
        }
    }
    fflush(stdout);
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main(void)
{
    putchar_entry();
    return 0;
}
