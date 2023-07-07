#include <stdio.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    const char *p1 = "abcdefghsd";
    const char *p2 = "def";

    char *ret = strstr(p1, p2);

    if (ret == NULL)
    {
        perror("ret NULL\n");
    }
    else
    {
        if (strcmp(ret, "defghsd"))
        {
            perror("strstr fail\n");
            return PTS_UNRESOLVED;
        }
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
