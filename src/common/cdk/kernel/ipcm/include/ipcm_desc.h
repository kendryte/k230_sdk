#ifndef __IPCM_DESC_H__
#define __IPCM_DESC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

#define READY_STATE_SHIFT       8
#define RESET_STATE_SHIFT       16

int is_node_ready(unsigned int nid);
int is_node_connected(unsigned int nid);
int ipcm_nodes_detecting(void *p);
void ipcm_node_release(int release);

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif
