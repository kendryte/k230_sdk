/* vsscanf example */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "posixtest.h"

void GetMatches(const char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsscanf(str, format, args);
    va_end(args);
}

int main()
{
    int val;
    char buf[100];
    char data[] = "abcd";
    int num = 1234;

    GetMatches("1234 abcd", " %d %s ", &val, buf);

    if (strcmp(buf, data))
    {
        perror("strcmp error");
        return PTS_UNRESOLVED;
    }

    if (val != num)
    {
        perror("val != num");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}