#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main()
{
    ldiv_t output;

    output = ldiv(100000L, 30000L);
    if (output.quot != 3 || output.rem != 10000)
    {
        perror("div error");
        return PTS_FAIL;
    }

    output = ldiv(2147483647L, 1112223334L);
    if (output.quot != 1 || output.rem != 1035260313)
    {
        perror("div error");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}