#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "posixtest.h"

static char buf[64] = {0};
static void WriteFrmtd(char *format, ...)
{
    va_list args;
    int res = 0;
    va_start(args, format);
    res = vsprintf(buf, format, args);
    if(res <= 0)
    {
        printf("Test failed\n");
        return PTS_FAIL;
    }
    va_end(args);
}

static int vsprintf_entry(void)
{
    char test_data[] = "vsprintf test:2021-8-1 3.14 0xff";
    WriteFrmtd("vsprintf test:%s-%d-%c %.02f 0x%x", "2021", 8, '1', 3.14, 0xff);

    if (strcmp(buf, test_data))
    {
        perror("strcmp error");
        return PTS_UNRESOLVED;
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main(void)
{
    vsprintf_entry();
    return 0;
}
