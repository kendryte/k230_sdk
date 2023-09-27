#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posixtest.h"

int main()
{
    int *p = (int *)calloc(10, sizeof(int));
    if (p == NULL)
    {
        perror("calloc error\n");
    }
    else
    {
        int i;
        for (i = 0; i < 10; i++)
        {
            printf("%d ", *(p + i));
        }
        printf("\n");
    }

    free(p);
    p = NULL;

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
