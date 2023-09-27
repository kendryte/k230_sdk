#include <stdio.h>
#include <string.h>
#include "posixtest.h"
int main(void)
{
    int a;
    char str[10];
    printf("please input string:\"hello 1234\"\n");
    scanf("%s %d", str, &a);
    printf("%s,%d\n",str, a);
    if (a != 1234 || strcmp(str,"hello"))
    {
        printf("Test Fail:wrong result\n");
        return PTS_FAIL;
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
