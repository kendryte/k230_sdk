
#include <fcntl.h>
#include <stdio.h>
#include "posixtest.h"

static int test_defined(void)
{
    int fd;

    fd = open("/fattest.txt", O_RDWR | O_CREAT, 0);

    if (fd == NULL)
    {
        perror("open error");
        return PTS_FAIL;
    }
    close(fd);

    printf("{Test PASSED}");
    return PTS_PASS;
}

int main(void)
{
    test_defined();
    return 0;
}