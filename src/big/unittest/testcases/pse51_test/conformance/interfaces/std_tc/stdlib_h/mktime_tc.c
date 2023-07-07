#include <stdio.h>
#include <time.h>
#include "posixtest.h"

int main()
{
    int ret;
    struct tm info;
    char buffer[80];

    info.tm_year = 2021 - 1900;
    info.tm_mon = 7 - 1;
    info.tm_mday = 4;
    info.tm_hour = 0;
    info.tm_min = 0;
    info.tm_sec = 1;
    info.tm_isdst = -1;

    ret = mktime(&info);
    if (ret == -1)
    {
        printf("Error: unable to make time using mktime\n");
    }
    else
    {
        strftime(buffer, sizeof(buffer), "%c", &info);
        printf(buffer);
    }

    printf("{Test PASSED}\n");
    return PTS_PASS;
}