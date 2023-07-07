#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include "posixtest.h"
bool checked_sscanf(int count, const char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int rc = vsscanf(buf, fmt, ap);
    va_end(ap);
    return rc == count;
}

int main(void)
{
    int n, m;
    char c;
    printf("Parsing '1 2'...\n");
    if (checked_sscanf(2, "1 2", "%d %d", &n, &m))
        puts("success");
    else
    {
        puts("failure");
        return PTS_FAIL;
    }

    printf("Parsing '1 a'...\n");
    if (checked_sscanf(2, "1 a", "%d %s", &n, &c))
        puts("success");
    else
    {
        puts("failure");
        return PTS_FAIL;
    }
    
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
