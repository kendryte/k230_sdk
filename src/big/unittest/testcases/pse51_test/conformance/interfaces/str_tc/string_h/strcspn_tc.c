#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main(void)
{
    int res1, res2, res3, res4;

    res1 = strcspn("abcde", "c");
    res2 = strcspn("abcdecde", "d");
    res3 = strcspn("abcde", "db");
    res4 = strcspn("abcde", "xyz");

    if (res1 == 2 && res2 == 3 && res3 == 1 && res4 == 5)
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }

    perror("strcspn fail\n");
    return PTS_UNRESOLVED;
}