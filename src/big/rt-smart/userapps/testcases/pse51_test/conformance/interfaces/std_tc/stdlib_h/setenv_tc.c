#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixtest.h"

int main(void)
{
    char *val;
    const char *name = "ABC";
    const char *str = "env test case";

    val = getenv(name);

    setenv(name, str, 1);

    val = getenv(name);
    if (strcmp(val, str))
    {
        perror("setenv error");
        return PTS_FAIL;
    }

    int ret = unsetenv(name);
    if (ret)
    {
        perror("unsetenv error");
        return PTS_FAIL;
    }

    val = getenv(name);
    if (val)
    {
        perror("getenv error");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}