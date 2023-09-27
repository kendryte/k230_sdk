#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	int pid = getpid();
	printf("Pleace use \'list_process\' to confirm that the process has exited\n");
	printf("This process(pid = %d) should have exited\n",pid);
	exit(0);
}
