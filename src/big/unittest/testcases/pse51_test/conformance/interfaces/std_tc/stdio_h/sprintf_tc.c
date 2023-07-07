#include <stdio.h>
#include <string.h>
#include "posixtest.h"

static int sprintf_entry(void)
{
    char buf[64] = {0};
    char test_data[] = "sprintf test:2021-8-1 3.14 0xff";
    sprintf(buf, "sprintf test:%s-%d-%c %.02f 0x%x", "2021", 8, '1', 3.14, 0xff);

    if (strcmp(buf, test_data))
    {
        perror("strcmp fail");
        return PTS_UNRESOLVED;
    }
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

int main(void)
{
    sprintf_entry();
    return 0;
}
