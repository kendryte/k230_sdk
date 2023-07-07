#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main()
{
    lldiv_t output;

    output = lldiv(100000L, 30000L);
    if (output.quot != 3 || output.rem != 10000)
    {
        perror("div error");
        return PTS_FAIL;
    }

    output = lldiv(9223372036854775807LL, 1112223334L);
    if (output.quot != 8292733801 || output.rem != 732063273)
    {
        perror("div error");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}