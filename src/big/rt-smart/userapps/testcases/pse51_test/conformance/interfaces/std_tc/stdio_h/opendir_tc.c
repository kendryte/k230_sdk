#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "posixtest.h"
static int count(DIR * thedir)
{
	int counter = 0;

	struct dirent *dp;

	rewinddir(thedir);

	/* Count the directory entries */

	do {
		dp = readdir(thedir);

		if (dp != NULL)
			counter++;
	}
	while (dp != NULL);

	return counter;
}

int main(void)
{
    int ret;
    int counted;
    DIR *dotdir;
    /* Open the directory */
	dotdir = opendir(".");
    if (dotdir == NULL) {
		printf("opendir failed\n");
        return PTS_FAIL;
	}

    /* Count entries */
	counted = count(dotdir);

    ret = closedir(dotdir);

	if (ret != 0) {
		printf("Failed to closedir in parent\n");
        return PTS_FAIL;
	}
    printf("{Test PASSED}\n");
    return PTS_PASS;
}

