#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "posixtest.h"

int strtol_entry(void)
{
    char buffer1[20] = "0x31da6c";
    char *stop1;
    assert(strtol(buffer1, &stop1, 0) == 3267180);
    assert(strcmp(stop1, "") == 0);

    char buffer2[20] = "0x31da6c";
    char *stop2;
    assert(strtol(buffer2, &stop2, 13) == 0);
    assert(strcmp(stop2, "x31da6c") == 0);

    char buffer_1[] = "10379c";
    char buffer_2[] = "      10379c        ";
    char buffer_3[] = "      10      379c  ";
    assert(strtol(buffer_1, NULL, 0) == 10379);
    assert(strtol(buffer_2, NULL, 0) == 10379);
    assert(strtol(buffer_3, NULL, 0) == 10);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main()
{
    strtol_entry();
    return 0;
}