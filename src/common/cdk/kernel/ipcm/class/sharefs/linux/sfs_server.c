
int sharefs_server_init(void)
{
	return sfs_ipc_init();
}

void sharefs_server_deinit(void)
{
	sfs_ipc_cleanup();
}
