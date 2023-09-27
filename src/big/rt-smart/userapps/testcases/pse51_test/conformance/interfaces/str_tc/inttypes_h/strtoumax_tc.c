
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "posixtest.h"

int main(void)
{
    int base = 10;
    char str[] = "999999abcdefg";
    char *end;
    uintmax_t num;

    num = strtoumax(str, &end, base);
    if (num != 999999)
    {
        perror("strtoumax1 error");
        return PTS_FAIL;
    }

    // in this case the end pointer points to null
    // here base change to char16
    base = 2;
    strcpy(str, "10010");
    num = strtoumax(str, &end, base);
    if (num != 18)
    {
        perror("strtoumax2 error");
        return PTS_FAIL;
    }

    if (*end)
    {
        perror("strtoumax3 error");
        return PTS_FAIL;
    }
    else
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }
}
