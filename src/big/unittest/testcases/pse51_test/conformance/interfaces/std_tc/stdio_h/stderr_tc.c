#include <stdio.h>

int main()
{
    int res = 0;
    res = fprintf(stderr, "Stderr Hello World!!\n");
    if(res <= 0)
    {
        printf("Test failed\n");
    }
    else 
        printf("{Test PASSED}\n");
    return 0;
}