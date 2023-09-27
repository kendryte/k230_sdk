#include <unistd.h>
#include <signal.h>

#include "sfs_server.h"

static int exiting = 0;
static void __exit(int sig)
{
	exiting = 1;
}

int main(int argc, char *argv[])
{
	sharefs_server_init();

	(void)signal(SIGINT, __exit);
	while (!exiting)
		sleep(1);
	sharefs_server_deinit();
	return 0;
}

