#include <stdarg.h>
#include <stdio.h>
#include "posixtest.h"

int sum(int, ...);

int main()
{
    if (sum(2, 15, 56) == 71)
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }

    perror("strtoimax error");
    return PTS_FAIL;
}

int sum(int num_args, ...)
{
    int val = 0;
    va_list ap;
    int i;

    va_start(ap, num_args);
    for (i = 0; i < num_args; i++)
    {
        val += va_arg(ap, int);
    }
    va_end(ap);

    return val;
}