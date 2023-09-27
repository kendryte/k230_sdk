#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "posixtest.h"

#define FILE_TEST_NAME "./rw_file_test.txt"
#define FILE_TEST_LENGTH(x) sizeof(x)

/* close read write fsync*/
static int open_read_write_fsync_close_entry(void)
{
    int res = 0;
    int size = 0;
    int fd = 0;
    char s[] = "RT-Thread Programmer!";
    char buffer[30];

    printf("the data is: %s\n", s);

    fd = open(FILE_TEST_NAME, O_RDWR | O_CREAT | O_APPEND);
    if (fd < 0)
    {
        perror("rw_entry error\n");
        return PTS_FAIL;
    }
    res = write(fd, s, FILE_TEST_LENGTH(s));
    if (res < 0)
    {
        perror("write error\n");
        return PTS_FAIL;
    }
    
    /* write -> read */
    lseek(fd,0,SEEK_SET);
    res = read(fd, buffer, FILE_TEST_LENGTH(s));
    if (res < 0)
    {
        perror("read error\n");
        return PTS_FAIL;
    }
    if (strcmp(buffer,s) != 0)
    {
        printf("read error : buffer not equal , \nthe buffer is: %s\nthe s is: %s\n", buffer,s);
    }

    /* write -> fsync -> read */
    if (fsync(fd) != 0)
    {
        perror("fync error\n");
        return PTS_FAIL;
    }
    lseek(fd,0,SEEK_SET);
    read(fd, buffer, FILE_TEST_LENGTH(s));
    if (strcmp(buffer,s) != 0)
    {
        printf("read error : buffer not equal , \nthe buffer is: %s\nthe s is: %s\n", buffer,s);
    }
    res = close(fd);
    if (res != 0)
    {
        perror("close error: ");
        return PTS_FAIL;
    }
    /* close -> open -> read */
    fd = open(FILE_TEST_NAME, O_RDONLY);
    if (fd >= 0)
    {
        size = read(fd, buffer, sizeof(s));
        printf("the buffer is: %s size:%d\n", buffer,size);
        close(fd);
        if (size < 0)
        {
            perror("read error\n");
            return PTS_FAIL;
        }
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main(void)
{
    open_read_write_fsync_close_entry();
    return 0;
}
