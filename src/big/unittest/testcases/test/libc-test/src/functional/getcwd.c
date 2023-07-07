#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "test.h"
#include <stdio.h>

static char path[256];
int main()
{
    char *p;

    p = getcwd(path, 256);
	if (p == NULL)
    {
		t_error("getcwd() returned %p\n", p);
    }
    else
    {
        printf("getcwd() returned %s\n", p);
    }
	return t_status;
}
