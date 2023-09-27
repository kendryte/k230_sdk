#include <stdio.h>
#include "posixtest.h"
#include <string.h>
int main()
{
    char buf[50];
    char str[] = "hello rt-smart\n";
    setbuf(stdout, buf);
    puts(str);
    if (strlen(buf) == NULL)
    {
        puts("test Fail\n");
        return PTS_FAIL;
    }
    fflush(stdout);
    puts(str);
    fflush(stdout);
    puts("{Test PASSED}\n");
    fflush(stdout);
    return PTS_PASS;
}
