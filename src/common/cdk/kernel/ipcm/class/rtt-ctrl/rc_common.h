#ifndef __RC_COMMON_H__
#define __RC_COMMON_H__

#define RC_CLIENT_ID    0
#define RC_SERVER_ID    1

#define RC_IPC_PORT    7

#define rc_error(s...)  do { \
	printf("\033[1;33m"); \
	printf("<s_fs#%s-%d#>:", __func__, __LINE__); \
	printf(s); \
	printf("\033[0m\n"); \
} while (0)

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif
