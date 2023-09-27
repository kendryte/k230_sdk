#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char str[] = "ab,cd,ef";
    const char *str_sp[] = {"ab", "cd", "ef"};
    char *ptr;
    char *save_p;

    ptr = strtok_r(str, ",", &save_p);

    int i;
    while (ptr != NULL)
    {
        if (strcmp(ptr, str_sp[i++]))
        {
            perror("strstr fail\n");
            return PTS_UNRESOLVED;
        }
        ptr = strtok_r(NULL, ",", &save_p);
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}