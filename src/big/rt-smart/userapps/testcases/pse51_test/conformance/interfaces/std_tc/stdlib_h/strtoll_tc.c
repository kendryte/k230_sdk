#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main()
{
    char szNumbers[] = "1856892505 17b00a12b -01100011010110000010001101100 0x6fffff";
    char *pEnd;
    long long int lli1, lli2, lli3, lli4;
    lli1 = strtoll(szNumbers, &pEnd, 10);
    lli2 = strtoll(pEnd, &pEnd, 16);
    lli3 = strtoll(pEnd, &pEnd, 2);
    lli4 = strtoll(pEnd, NULL, 0);

    if (lli1 == 1856892505LL && lli2 == 6358606123LL && lli3 == -208340076LL && lli4 == 7340031LL)
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }

    perror("strtoll error\n");
    return PTS_FAIL;
}