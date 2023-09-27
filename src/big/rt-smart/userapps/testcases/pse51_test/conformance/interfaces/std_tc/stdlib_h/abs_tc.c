#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main()
{
    int a = 0, b = -2147483647, c, d;
    c = abs(a);
    d = abs(b);

    if (c == 0 && d == 2147483647)
    {
        printf("{Test PASSED}\n");
    }
    else
    {
        perror("abs fail\n");
        return PTS_UNRESOLVED;
    }
    return PTS_PASS;
}