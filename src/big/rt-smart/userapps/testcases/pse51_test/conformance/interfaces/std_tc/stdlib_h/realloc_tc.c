#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char *str = NULL;
    char *str_cpy = "rtthread.com";

    str = (char *)malloc(10);
    strcpy(str, "rtthread");

    str = (char *)realloc(str, 25);
    if (str == NULL)
    {
        perror("realloc error\n");
        return PTS_FAIL;
    }

    strcat(str, ".com");

    if (strcmp(str, str_cpy))
    {
        perror("strcmp error\n");
        free(str);
        return PTS_FAIL;
    }

    free(str);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}