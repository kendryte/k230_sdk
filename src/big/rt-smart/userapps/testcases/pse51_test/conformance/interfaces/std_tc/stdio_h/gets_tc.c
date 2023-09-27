#include <stdio.h>
#include <string.h>
#include "posixtest.h"
int main()
{
    char str[50];

    printf("please input string with a length less than 50: \n");
    int ret = gets(str);
    if(ret == NULL)
    {
        printf("Test Fail\n");
        return PTS_FAIL;
    }
    if(strlen(str) == NULL)
    {
        printf("Test Fail\n");
        return PTS_FAIL;
    }
    printf("%s", str);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
