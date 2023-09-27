#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "posixtest.h"

static void WriteFrmtd(char *format, ...)
{
    va_list args;
    int res = 0;
    va_start(args, format);
    res = vprintf(format, args);
    if(res <= 0)
    {
        printf("Test failed\n");
        return PTS_FAIL;
    }
    va_end(args);
}

static int vprintf_entry(void)
{
    WriteFrmtd("vprintf test:%s-%d-%c %.02f 0x%x\n", "2021", 8, '1', 3.14, 0xff);
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main(void)
{
    vprintf_entry();
    return 0;
}
