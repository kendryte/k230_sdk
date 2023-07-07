#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "posixtest.h"

#define TIMES (100)
int main()
{
    int i;
    time_t t;
    int buf1[TIMES] = {};
    int buf2[TIMES] = {};
    //verify rand()
    srand(TIMES);

    for (i = 0; i < TIMES; i++)
    {
        buf1[i] = rand() % 50;
    }

    srand(TIMES);

    for (i = 0; i < TIMES; i++)
    {
        buf2[i] = rand() % 50;
    }

    for (i = 0; i < TIMES; i++)
    {
        printf("rand buf1:buf2 [%d : %d]\n",buf1[i] ,buf2[i]);
        if(buf1[i] != buf2[i])
        {
            printf("{Test FAIL} : same seed but ret not same");
            return PTS_FAIL;
        }
    }

    //verify srand()
    srand(TIMES*2);
    for (i = 0; i < TIMES; i++)
    {
        buf2[i] = rand() % 50;
    }
    int difcount = 0;
    for (i = 0; i < TIMES; i++)
    {
        printf("rand buf1:buf2 [%d : %d]\n",buf1[i] ,buf2[i]);
        if(buf1[i] != buf2[i])
            difcount++;
    }
    if(difcount<=0){
        printf("{Test FAIL}\n");
        return PTS_FAIL;
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}