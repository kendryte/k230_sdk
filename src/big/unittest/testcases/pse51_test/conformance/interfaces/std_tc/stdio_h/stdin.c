#include <stdio.h>
#include "posixtest.h"
int main()
{
    printf("please input char: \n");
    char c = getc_unlocked(stdin);
    if(c == NULL)
    {
        printf("Test fail : getc_unlocked return 0\n");
        return PTS_FAIL;
    }
    putc_unlocked(c, stdout);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
