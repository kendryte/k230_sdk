#ifndef MY_DATA_SHM_H_
#define MY_DATA_SHM_H_

#include <unistd.h>
#include <cstdint>

// data-shm utils
void *user_sys_mmap(uint64_t phys_addr, size_t size);
int user_sys_munmap(uint64_t phys_addr, void *virt_addr, size_t size);

void user_sys_flush_cache(void *start, size_t size);
void user_sys_invalidate_cache(void *start, size_t size);

#endif // MY_DATA_SHM_H_
