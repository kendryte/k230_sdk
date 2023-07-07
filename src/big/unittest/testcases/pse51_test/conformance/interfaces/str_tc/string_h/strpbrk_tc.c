#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    const char str1[] = "abcde2fghi3jk4l";
    const char str2[] = "34";
    char *ret;

    ret = strpbrk(str1, str2);
    if (ret)
    {
        if (*ret != 51)
        {
            perror("strpbrk fail\n");
            return PTS_UNRESOLVED;
        }
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}