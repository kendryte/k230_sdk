#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main()
{
    div_t output;

    output = div(27, 4);
    if (output.quot != 6 || output.rem != 3)
    {
        perror("div error");
        return PTS_FAIL;
    }

    output = div(27, 3);
    if (output.quot != 9 || output.rem != 0)
    {
        perror("div error");
        return PTS_FAIL;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}