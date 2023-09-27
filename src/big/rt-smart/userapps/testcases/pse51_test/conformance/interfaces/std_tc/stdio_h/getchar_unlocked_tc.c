#include <stdio.h>
#include "posixtest.h"
int main()
{
    printf("please input char: \n");
    char c = getchar_unlocked();
    if(c == NULL)
    {
        printf("Test fail : getc_unlocked return 0\n");
        return PTS_FAIL;
    }
    putchar_unlocked(c);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
