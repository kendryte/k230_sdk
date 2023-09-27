#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"
int main()
{
    char buff[1024];

    memset(buff, '\0', sizeof(buff));

    fprintf(stdout, "enable full buffering\n");
    int ret = setvbuf(stdout, buff, _IOFBF, 1024);
    if(ret)
    {
        puts("test Fail\n");
        return PTS_FAIL;
    }
    fprintf(stdout, "hello rt-smart\n");
    fprintf(stdout, "This output will be saved to buff\n");
    fflush(stdout);

    fprintf(stdout, "This will appear when programming\n");
    fprintf(stdout, "Sleep for a second at the end\n");

    sleep(1);
    // Here, the program saves the buffered output to the buff until fflush()
    // is first called, then starts buffering output, and finally sleeps 1 second. 
    // It sends the rest of the output to STDOUT before the program ends.
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
