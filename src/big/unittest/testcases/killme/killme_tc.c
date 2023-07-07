#include <stdio.h>
#include <rtthread.h>

int main()
{
    printf("Please kill me.\n");

    while (1)
    {
        rt_thread_mdelay(100);
    }

    return 0;
}
