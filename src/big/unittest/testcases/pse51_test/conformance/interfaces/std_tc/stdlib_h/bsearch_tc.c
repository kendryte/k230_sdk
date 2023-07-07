#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int values[] = {5, 20, 29, 32, 63};

int main()
{
    int *item;
    int key = 32;

    item = (int *)bsearch(&key, values, 5, sizeof(int), cmpfunc);
    if (item != NULL)
    {
        printf("{Test PASSED}\n");
    }
    else
    {
        perror("bsearch error\n");
    }

    return PTS_PASS;
}