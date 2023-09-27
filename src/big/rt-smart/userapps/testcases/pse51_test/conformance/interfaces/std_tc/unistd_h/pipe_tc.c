#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define MSG_SIZE 16
const char *msg1 = "hello, world #1";
const char *msg2 = "hello, world #2";
const char *msg3 = "hello, world #3";

int main()
{
    char inbuf[MSG_SIZE];
    int  p[2], i;

    // create pipe
    if (pipe(p) < 0)
    {
        printf("create pipe Failed");
        return PTS_FAIL;
    }

    // write pipe
    write(p[1], msg1, MSG_SIZE);
    write(p[1], msg2, MSG_SIZE);
    write(p[1], msg3, MSG_SIZE);

    for (i = 0; i < 3; i++) {
        // read pipe
        read(p[0], inbuf, MSG_SIZE);
        printf("%s\n", inbuf);
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
