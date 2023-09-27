#include <stdarg.h>
#include <stdio.h>
#include "posixtest.h"
int checked_vfscanf(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vfscanf(stream, format, args);
    va_end(args);
    return ret;
}

int main()
{
    int  val;
    char str[100];

    printf("please input alt string (like: \"hello 1234\"), Use \"alt + enter\" to confirm: \n");
    int ret = checked_vfscanf(stdin, " %s %d ", str, &val);
    if(ret == EOF)
    {
        printf("Test Fail\n");
        return PTS_FAIL;
    }
    printf("I have read %s and %d \n", str, val);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
