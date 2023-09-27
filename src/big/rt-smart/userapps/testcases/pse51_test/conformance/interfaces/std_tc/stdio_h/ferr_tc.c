#include <stdio.h>
#include "posixtest.h"

int main()
{
	FILE *fp;
	char c;

	fp = fopen("/sdcard/file.txt", "w");

	c = fgetc(fp);
	if (ferror(fp))
	{
		printf("cannot reading from file : file.txt\n");
	}
	else
	{
		printf("Test Fail\n");
		return PTS_FAIL;
	}
	clearerr(fp);

	if (ferror(fp))
	{
		printf("Test Fail: still have error code\n");
		return PTS_FAIL;
	}
	fclose(fp);
    printf("{Test PASSED}\n");
	return PTS_PASS;
}