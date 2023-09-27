#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    int len;
    const char str1[] = "ABCDEFG019874";
    const char str2[] = "ABCD";

    len = strspn(str1, str2);

    if (len != 4)
    {
        perror("strspn fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}