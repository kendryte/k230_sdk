#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main(void)
{
    char dest[] = "oldstring";
    const char src[] = "newstring";

    memmove(dest, src, 9);
    if (strcmp(dest, src))
    {
        perror("memmove fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}