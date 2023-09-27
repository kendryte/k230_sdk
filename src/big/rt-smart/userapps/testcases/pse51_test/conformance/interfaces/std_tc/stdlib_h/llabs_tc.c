#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main()
{
    long long int a, b;

    a = llabs(9223372036854775807LL);
    b = llabs(-9223372036854775807LL - 1) - 1;
    printf("a = %lld\n", a);
    printf("b = %lld\n", b);
    if (a != 9223372036854775807LL || b != 9223372036854775807LL)
    {
        perror("labs error");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}