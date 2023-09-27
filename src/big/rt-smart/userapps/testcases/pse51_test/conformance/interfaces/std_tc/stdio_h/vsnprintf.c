#include <stdarg.h>
#include <stdio.h>
#include "posixtest.h"
char buffer[256];
int  vspfunc(char *format, ...)
{
    va_list aptr;
    int     ret;

    va_start(aptr, format);
    ret = vsnprintf(buffer, 256, format, aptr);
    printf(buffer);
    va_end(aptr);

    return (ret);
}

int main()
{
    int   i       = 5;
    float f       = 27.0;
    char  str[50] = "hello";

    int ret = vspfunc("%d %f %s", i, f, str);
    if(ret<=0)
    {
        printf("Test Fail\n");
        return PTS_FAIL;
    }
    printf("%s\n", buffer);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
