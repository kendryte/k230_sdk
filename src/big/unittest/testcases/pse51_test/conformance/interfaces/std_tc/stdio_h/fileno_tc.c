#include <stdio.h>
#include "posixtest.h"

static int fdopen_entry(void)
{
    int stdin_no = fileno(stdin);
    int stdout_no = fileno(stdout);
    int stderr_no = fileno(stderr);

    if ((stdin_no == 0) && (stdout_no == 1) && (stderr_no == 2))
    {
        printf("{Test PASSED}\n");
        return PTS_PASS;
    }
    perror("Test fail\n");
    return PTS_UNRESOLVED;
}

int main(void)
{
    fdopen_entry();
    return 0;
}
