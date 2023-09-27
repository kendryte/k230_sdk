#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include "test.h"

#define TEST(c, ...) ((c) ? 1 : (t_error(#c" failed: \n" __VA_ARGS__),0))
#define TESTE(c) (errno=0, TEST(c, "errno = %s\n", strerror(errno)))
#define BUF_SIZE 10
int main(void)
{
	struct flock fl = {0};
	FILE *f;
	int fd;
	pid_t pid;
	int status;
	int flag;
	char *temp_file = "/sdcard/temp_file_fcntl";
	// char *s = "hello world!";
	unlink(temp_file);
	TEST((fd = open(temp_file,O_RDWR|O_CREAT|O_EXCL)) > 2);
	//test F_DUPFD

	//test whether fd will be close after exec() function???
	//1.fock() sub process
	//2. sub process call exec()
	//3. exec() process read fd,expect error EBADFD
	//4. parent process can read fd of course.

	//test F_GETFL
	TEST((flag = fcntl(fd,F_GETFL)) != -1);
	TEST(O_RDWR == (flag&O_ACCMODE));

	//test F_SETFL
	flag = flag | O_APPEND;
	TEST(fcntl(fd,F_SETFL,flag) != -1);
	TEST((fcntl(fd,F_GETFL)&O_APPEND) == O_APPEND);

	unlink(temp_file);

// #define F_GETLK 12
// #define F_SETLK 13

	if (!TESTE(f=fopen(temp_file,"wb+"))) return t_status;
	fd = fileno(f);

	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	TESTE(fcntl(fd, F_SETLK, &fl)==0);

	pid = fork();
	if (!pid) {
		fl.l_type = F_WRLCK;
		_exit(fcntl(fd, F_GETLK, &fl) || fl.l_pid != getppid());
	}
	while (waitpid(pid, &status, 0)<0 && errno==EINTR);
	TEST(status==0, "child failed to detect lock held by parent\n");

	fclose(f);
	printf("test finished t_status : %d\n",t_status);
	return t_status;
}
