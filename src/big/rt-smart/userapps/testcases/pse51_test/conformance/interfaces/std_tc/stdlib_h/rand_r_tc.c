#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "posixtest.h"

#define TIMES (100)
int main()
{
    int i;
    unsigned int seed = 123;
    int buf[TIMES] = {};
    int count = 0;
    for (i = 0; i < TIMES; i++)
    {
        buf[i] = rand_r(&seed);
        printf("%d\n", buf[i]);
    }

    for (i = 0; i < TIMES; i++)
    {
        if (buf[0] == buf[i])
        {
            count++;
        }
    }
    printf("same count = %d\n", count);
    if (count==TIMES)
    {
        printf("{Test FAIL}\n");
        return PTS_FAIL;
    }
    
    printf("{Test PASSED}\n");
    return PTS_PASS;
}