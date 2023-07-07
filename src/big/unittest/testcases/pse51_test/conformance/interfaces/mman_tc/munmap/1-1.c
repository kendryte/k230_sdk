#include <stdio.h>
#include <sys/mman.h>
#include "posixtest.h"
int main(int argc, char **argv)
{
    char  *addr;
    size_t len = 4096;

    addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (addr == MAP_FAILED) {
        printf("mmap Fail");
        return PTS_FAIL;
    }

    sprintf(addr, "Hello, world!");

    printf("%s\n", addr);

    if (munmap(addr, len) == -1) {
        perror("munmap Fail");
        return PTS_FAIL;
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
