#ifndef __SHARE_FS_COMMON_H__
#define __SHARE_FS_COMMON_H__

/* configuration by user  begin */
/* the clients which should be connected
 * 0x01 : enable
 * 0x00 : disable
 * else : error */
#define SFS_CLIENT_ID_0_EN   0x00
#define SFS_CLIENT_ID_1_EN   0x01
#define SFS_CLIENT_ID_2_EN   0x00
#define SFS_CLIENT_ID_3_EN   0x00
#define SFS_CLIENT_ID_4_EN   0x00
#define SFS_CLIENT_ID_5_EN   0x00
#define SFS_CLIENT_ID_6_EN   0x00
#define SFS_CLIENT_ID_7_EN   0x00
/* server */
#define SFS_SERVER_ID    0
/* port */
#define SFS_IPC_PORT    6
/* configuration by user  end */

#define SFS_CLIENTS  ((SFS_CLIENT_ID_0_EN << 0) \
		| (SFS_CLIENT_ID_1_EN << 1) \
		| (SFS_CLIENT_ID_2_EN << 2) \
		| (SFS_CLIENT_ID_3_EN << 3) \
		| (SFS_CLIENT_ID_4_EN << 4) \
		| (SFS_CLIENT_ID_5_EN << 5) \
		| (SFS_CLIENT_ID_6_EN << 6) \
		| (SFS_CLIENT_ID_7_EN << 7))
#define SFS_CLIENTS_CNT  ((SFS_CLIENT_ID_0_EN) \
		+ (SFS_CLIENT_ID_1_EN) \
		+ (SFS_CLIENT_ID_2_EN) \
		+ (SFS_CLIENT_ID_3_EN) \
		+ (SFS_CLIENT_ID_4_EN) \
		+ (SFS_CLIENT_ID_5_EN) \
		+ (SFS_CLIENT_ID_6_EN) \
		+ (SFS_CLIENT_ID_7_EN))
#if ((1 << (SFS_SERVER_ID)) & (SFS_CLIENTS))
#error "<share fs> configuration conflicted"
#endif

#if ((((SFS_CLIENT_ID_0_EN) != 0x00)&&((SFS_CLIENT_ID_0_EN) != 0x01)) \
		|| (((SFS_CLIENT_ID_1_EN) != 0x00)&&((SFS_CLIENT_ID_1_EN) != 0x01)) \
		|| (((SFS_CLIENT_ID_2_EN) != 0x00)&&((SFS_CLIENT_ID_2_EN) != 0x01)) \
		|| (((SFS_CLIENT_ID_3_EN) != 0x00)&&((SFS_CLIENT_ID_3_EN) != 0x01)) \
		|| (((SFS_CLIENT_ID_4_EN) != 0x00)&&((SFS_CLIENT_ID_4_EN) != 0x01)) \
		|| (((SFS_CLIENT_ID_5_EN) != 0x00)&&((SFS_CLIENT_ID_5_EN) != 0x01)) \
		|| (((SFS_CLIENT_ID_6_EN) != 0x00)&&((SFS_CLIENT_ID_6_EN) != 0x01)) \
		|| (((SFS_CLIENT_ID_7_EN) != 0x00)&&((SFS_CLIENT_ID_7_EN) != 0x01)))
#error "<share fs> configuration error"
#endif

#ifdef __LITEOS__
#define printf dprintf
#endif

#define sfs_error(s...)  do { \
	printf("\033[1;33m"); \
	printf("<s_fs#%s-%d#>:", __func__, __LINE__); \
	printf(s); \
	printf("\033[0m\n"); \
} while (0)

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif
