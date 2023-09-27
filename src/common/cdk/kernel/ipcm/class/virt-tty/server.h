#ifndef __VIRT_TTY_SERVER_HEADER__
#define __VIRT_TTY_SERVER_HEADER__

int virt_tty_server_init(void);
void virt_tty_server_cleanup(void);
int virt_tty_server_read(void *data, void *buf, unsigned int len);
int virt_tty_server_write(void *data, void const *buf, unsigned int len);

#endif
