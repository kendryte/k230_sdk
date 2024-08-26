#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>

#define MAX_FILE_NAME  64
#define BUF_LEN    256
#define READ_BUF_LEN    16
/*-----------------------------------------------*/
#include <termios.h>

struct termios raw, set;

/* Set terminal i/o settings */
static void init_termios(void)
{
	tcgetattr(STDIN_FILENO, &raw);
	memcpy(&set, &raw, sizeof(struct termios));
	set.c_lflag &= ~ICANON; /* disable buffered i/o */
	set.c_lflag &= ~ECHO; /* set echo mode */
	tcsetattr(STDIN_FILENO, TCSANOW, &set);
}

/* Restore raw terminal i/o settings */
static void reset_termios(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}
/*------------------------------------------------*/
static int fd;
static int exiting = 0;

static void *virt_tty_read(void *arg)
{
	char buffer[BUF_LEN];
	ssize_t read_len;

	memset(buffer, 0x00, BUF_LEN);
	while (1) {
		read_len = read(fd, buffer, BUF_LEN);
		if (read_len > 0)
			write(STDOUT_FILENO, buffer, read_len);
		usleep(20000);
		if (exiting)
			break;
	}
	pthread_exit(0);
	return NULL;
}

static void __exit(int sig)
{
	exiting = 1;
}

int main(int argc, char *argv[])
{
	char buf[READ_BUF_LEN];
	fd_set rd_fds;
	ssize_t read_len;
	struct timeval timeout;
	pthread_t thread;
	void *exit;
	char file[MAX_FILE_NAME];

	if (argc != 2) {
		printf("Usage: virt_tty <device>\n");
		printf("   eg. virt_tty m7\n");
		printf("<device> is the device node in /dev/virt-tty\n");
		return 0;
	}
	snprintf(file, MAX_FILE_NAME, "/dev/virt-tty/%s", argv[1]);
	fd = open(file, O_RDWR);
	if (fd < 0) {
		printf("open %s failed.\n", file);
		return -1;
	}
	if (pthread_create(&thread, NULL, virt_tty_read, NULL)) {
		printf("create pthread failed.\n");
		close(fd);
		return -1;
	}
	init_termios();
	(void)signal(SIGINT, __exit);
	while (1) {
		FD_ZERO(&rd_fds);
		FD_SET(STDIN_FILENO, &rd_fds);
		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;
		memset(buf, 0x00, READ_BUF_LEN);
		(void)select(STDIN_FILENO + 1, &rd_fds, NULL, NULL, &timeout);
		if (FD_ISSET(STDIN_FILENO, &rd_fds)) {
			read_len = read(STDIN_FILENO, buf, READ_BUF_LEN);
			if (read_len > 0) {
				if ((1 == read_len) && ('\n' == buf[0]))
					write(fd, "\r", 1);
				else
					write(fd, buf, read_len);
			}
		}
		if (exiting)
			break;
	}
	pthread_join(thread, &exit);
	reset_termios();
	close(fd);
	return 0;
}
