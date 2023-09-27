#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char str[50];
    const char *str_cmp = "$$$$$$$ rtthread";

    strcpy(str, "This is rtthread");
    memset(str, '$', 7);

    if (strcmp(str, str_cmp))
    {
        perror("memmove fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}