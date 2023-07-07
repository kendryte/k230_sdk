#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "posixtest.h"

#define EPSILON 0.01

int main()
{
    char szOrbits[] = "686.97 365.24";
    char *pEnd;
    float f1, f2;

    f1 = strtof(szOrbits, &pEnd);
    f2 = strtof(pEnd, NULL);

    if (fabs(f1 - 686.97) < EPSILON && fabs(f2 - 365.24) < EPSILON)
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }

    perror("strtof error");
    return PTS_FAIL;
}