#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

//This code needs to be called with msh/> access.elf filename for parameter passing
int main(int argc, char *argv[])
{
    char filename[] = "access_tc";
    printf("filename %s\n",filename);
    int rt_value;
    rt_value = access(filename, R_OK);
    if (rt_value == 0)
        printf("File:%s can read   rt_value=%d\n", argv[1], rt_value);
    else
        printf("File:%s can't read  rt_value=%d \n", argv[1], rt_value);

    rt_value = access(filename, W_OK);
    if (rt_value == 0)
        printf("File:%s can write   rt_value=%d\n", argv[1], rt_value);
    else
        printf("File:%s can't write  rt_value=%d \n", argv[1], rt_value);

    rt_value = access(filename, X_OK);
    if (rt_value == 0)
        printf("File:%s can execute   rt_value=%d\n", argv[1], rt_value);
    else
        printf("File:%s can't execute  rt_value=%d \n", argv[1], rt_value);

    rt_value = access(filename, F_OK);
    if (rt_value == 0)
        printf("File:%s   exist   rt_value=%d\n", argv[1], rt_value);
    else
        printf("File:%s not exist  rt_value=%d \n", argv[1], rt_value);

    printf("{Test PASSED}\n");
    return PTS_PASS;
}
