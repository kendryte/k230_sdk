#ifndef __VIRT_TTY_CLIENT_HEADER__
#define __VIRT_TTY_CLIENT_HEADER__

int virt_tty_client_init(void);
void virt_tty_client_cleanup(void);
int virt_tty_client_read(void *buf, unsigned int len);
int virt_tty_client_send(void const *buf, unsigned int len);
int virt_tty_client_received(void);

#endif
