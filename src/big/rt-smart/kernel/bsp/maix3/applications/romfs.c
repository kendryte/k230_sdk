#include <dfs_romfs.h>

const static unsigned char _bin_test_kmodel[] = {
0x6b,0x0a,
};

const static unsigned char _bin_fastboot_app_elf[] = {
0x61,0x0a,
};

const static unsigned char _bin_init_sh[] = {
0x2f,0x62,0x69,0x6e,0x2f,0x66,0x61,0x73,0x74,0x62,0x6f,0x6f,0x74,0x5f,0x61,0x70,
0x70,0x2e,0x65,0x6c,0x66,0x20,0x2f,0x62,0x69,0x6e,0x2f,0x74,0x65,0x73,0x74,0x2e,
0x6b,0x6d,0x6f,0x64,0x65,0x6c,0x0a,
};

const static struct romfs_dirent _bin[] = {
	{ROMFS_DIRENT_FILE, "test.kmodel", (char*)0x1fc00000, 0x9ff80},
	{ROMFS_DIRENT_FILE, "fastboot_app.elf", (char*)0x18000000, 8677592},
	{ROMFS_DIRENT_FILE, "init.sh", _bin_init_sh, sizeof(_bin_init_sh)},
};

const struct romfs_dirent _root_dirent[] = {
#ifdef RT_USING_DFS_DEVFS
	{ROMFS_DIRENT_DIR, "dev", RT_NULL, 0},
#endif
#ifdef RT_USING_SDIO
	{ROMFS_DIRENT_DIR, "sdcard", RT_NULL, 0},
#endif
#ifdef RT_USING_PROC
	{ROMFS_DIRENT_DIR, "proc", RT_NULL, 0},
#endif
#ifdef RT_USING_DFS_SHAREFS
	{ROMFS_DIRENT_DIR, "sharefs", RT_NULL, 0},
#endif
	{ROMFS_DIRENT_DIR, "bin", (rt_uint8_t*) _bin, sizeof(_bin)/sizeof(_bin[0])},
};

const struct romfs_dirent romfs_root = {ROMFS_DIRENT_DIR, "/", (rt_uint8_t*) _root_dirent, sizeof(_root_dirent)/sizeof(_root_dirent[0])};

