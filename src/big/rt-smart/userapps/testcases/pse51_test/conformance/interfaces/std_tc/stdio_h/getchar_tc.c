#include <stdio.h>
#include "posixtest.h"
int main()
{
    printf("please input char: \n");
    char c = getchar();
    if(c == NULL)
    {
        printf("Test fail : getchar return 0\n");
        return PTS_FAIL;
    }
    putchar(c);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
