#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char str[] = "10 50 f 100 ";
    char *end;
    intmax_t a, b, c, d;

    // at base 10
    a = strtoimax(str, &end, 10);
    // at base 8
    b = strtoimax(end, &end, 8);
    // at base 16
    c = strtoimax(end, &end, 16);
    // at base 2
    d = strtoimax(end, &end, 2);

    if (a == 10 && b == 40 && c == 15 && d == 4)
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }

    perror("strtoimax error");
    return PTS_FAIL;
}
