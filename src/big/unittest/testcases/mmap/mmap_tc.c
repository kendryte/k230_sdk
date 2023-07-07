#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    void *pv;

    fd = open("/dev/uart", 0, O_RDONLY);
    if (fd < 0)
    {
        printf("dev file open failure\n");
        return -1;
    }

    pv = mmap(NULL, 0x400, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pv != NULL)
    {
        printf("Line Control Reg value:%02x\n", *(unsigned char*)(pv + 0x0c));
    }
    else
    {
        printf("mmap failure\n");
    }

    close(fd);

    return 0;
}
