#include <stdarg.h>
#include <stdio.h>
#include "posixtest.h"
int checked_vfprintf(FILE *stream, char *format, ...)
{
    va_list args;

    va_start(args, format);
    int ret = vfprintf(stream, format, args);
    va_end(args);
    return ret;
}

int main()
{
    int ret = checked_vfprintf(stdout, "This is just one argument %d \n", 10);
    if(ret<=0)
    {
        printf("Test fail\n");
        return PTS_FAIL;
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
