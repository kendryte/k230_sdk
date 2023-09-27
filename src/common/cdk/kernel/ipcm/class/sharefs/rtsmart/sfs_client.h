#ifndef __SHARE_FS_CLIENT_H__
#define __SHARE_FS_CLIENT_H__

int sharefs_client_init(const char *path);
void sharefs_client_deinit(const char *path);
int sharefs_client_connected(void);
#endif
