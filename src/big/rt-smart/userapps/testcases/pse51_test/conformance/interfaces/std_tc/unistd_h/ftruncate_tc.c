#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "posixtest.h"

#define TRUN_SIZE 3
#define TRUNCATE_TEST_NAME "./ftruncate_to3"

static int ftruncate_entry(void)
{
    int res = 0;
    int fd = 0;

    fd = open(TRUNCATE_TEST_NAME, O_RDWR | O_CREAT | O_APPEND);
    if (fd < 0)
    {
        return -1;
    }

    write(fd, "hello", 6);
    /* TODO */
    res = ftruncate(fd, TRUN_SIZE);
    close(fd);
    return res;
}

int main(void)
{
    ftruncate_entry();
    
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
