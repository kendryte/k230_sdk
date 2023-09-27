#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"
int main()
{
    DIR           *dir;
    struct dirent *ptr;
    dir = opendir(".");
    while ((ptr = readdir(dir)) != NULL) {
        printf("d_name : %s\n", ptr->d_name);
    }
    closedir(dir);
    dir = opendir(".");
    rewinddir(dir);
    if(dir == NULL)
    {
        printf("Test Fail\n");
        return PTS_FAIL;
    }
    printf("readdir again!\n");
    while ((ptr = readdir(dir)) != NULL) {
        printf("d_name : %s\n", ptr->d_name);
    }
    closedir(dir);
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
