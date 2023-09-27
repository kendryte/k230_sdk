#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main()
{
    long int a, b;

    a = labs(2147483647L);
    b = labs(-2147483647L - 1) - 1;
    printf("a = %ld\n", a);
    printf("b = %ld\n", b);
    if (a != 2147483647L || b != 2147483647L)
    {
        perror("labs error");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}