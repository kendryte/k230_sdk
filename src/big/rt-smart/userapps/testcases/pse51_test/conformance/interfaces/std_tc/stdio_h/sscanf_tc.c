#include <stdio.h>
#include <string.h>
#include "posixtest.h"

static int sscanf_entry(void)
{
    int day, year;
    char weekday[20], month[20], dtm[100];

    strcpy(dtm, "Friday January 1 2021");
    sscanf(dtm, "%s %s %d  %d", weekday, month, &day, &year);

    if (strcmp(month, "January") || strcmp(weekday, "Friday") || (year != 2021) || (day != 1))
    {
        perror("strcmp fail");
        return PTS_UNRESOLVED;
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main(void)
{
    sscanf_entry();
    return 0;
}
