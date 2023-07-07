#ifndef  __LWP_SHM_H__
#define  __LWP_SHM_H__

#include <stddef.h>

int lwp_shmget(size_t key, size_t size, int create);
int lwp_shmrm(int id);
void* lwp_shmat(int id, void* shm_vaddr);
int lwp_shmdt(void* shm_vaddr);

#endif  /*__LWP_SHM_H__*/
