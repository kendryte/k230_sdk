#include <stdio.h>
#include "posixtest.h"

static int fflush_entry(void)
{
    int res = 0;
    res = fflush(stdout);
    if(res != 0)
    {
        printf("Test failed\n");
        return PTS_FAIL;
    }
    printf("test fflush\n");

    printf("t");
    printf("e");
    printf("s");
    printf("t");
    res = fflush(stdout);
    if(res != 0)
    {
        printf("Test failed\n");
        return PTS_FAIL;
    }
    printf(" fflush");
    res = fflush(stdout);
    if(res != 0)
    {
        printf("Test failed\n");
        return PTS_FAIL;
    }
    printf("\n");
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main(void)
{
   fflush_entry();
   return 0;
}
