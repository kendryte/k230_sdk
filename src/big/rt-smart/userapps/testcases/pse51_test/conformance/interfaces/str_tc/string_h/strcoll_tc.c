#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    char str1[15];
    char str2[15];
    int ret;

    strcpy(str1, "abc");
    strcpy(str2, "ABC");

    ret = strcoll(str1, str2);

    if (ret <= 0)
    {
        perror("strcoll fail\n");
        return PTS_UNRESOLVED;
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}