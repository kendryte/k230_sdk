#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "posixtest.h"
bool checked_scanf(int count, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int rc = vscanf(fmt, ap);
    va_end(ap);
    return rc == count;
}

int main(void)
{
    int a;
    char str[10];
    printf("please input string:\"hello 1234\"\n");
    if (!checked_scanf(2,"%s %d", str, &a))
    {
        printf("Test Fail:wrong return\n");
        return PTS_FAIL;
    }
    if (a != 1234 || strcmp(str,"hello"))
    {
        printf("Test Fail:wrong result\n");
        return PTS_FAIL;
    }
    
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
