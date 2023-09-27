#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "test.h"

#define TEST(c) do { \
	errno = 0; \
	if (!(c)) \
		t_error("%s failed (errno = %d)\n", #c, errno); \
} while(0)

int main(void)
{
	char tmp[] = "/sdcard/temp_file_fdopen";
	char foo[6];
	int fd;
	FILE *f;
	unlink(tmp);
	TEST((fd = open(tmp,O_RDWR|O_CREAT|O_EXCL)) > 2);
	TEST(write(fd, "hello", 6)==6);
	TEST(f = fdopen(fd, "wb+"));
	if (f) {
		TEST(ftello(f)==6);
		TEST(fseeko(f, 0, SEEK_SET)==0);
		TEST(fgets(foo, sizeof foo, f));
		if (strcmp(foo,"hello") != 0)
			t_error("fgets read back: \"%s\"; wanted: \"hello\"\n", foo);
		fclose(f);
	}
	else
		t_error("tmpfile is NULL\n");

	return t_status;
}
