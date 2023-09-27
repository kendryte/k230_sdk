#include <stdio.h>
#include "posixtest.h"
int main()
{
    printf("please input char: \n");
    char c = getc(stdin);
    if(c == NULL)
    {
        printf("Test fail : getc_unlocked return 0\n");
        return PTS_FAIL;
    }
    int ret = putc(c, stdout);
    if(ret == EOF)
    {
        printf("Test fail\n");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
