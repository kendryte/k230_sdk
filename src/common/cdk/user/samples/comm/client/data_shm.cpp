#include <mutex>
#include <fcntl.h>
#include <sys/mman.h>
#include "data_shm.h"

static int g_mmap_fd_tmp = 0;
static std::mutex mmap_mutex_;
void *user_sys_mmap(uint64_t phys_addr, size_t size)
{
    void *virt_addr = NULL;
    void *mmap_addr = NULL;
    uint32_t page_size = sysconf(_SC_PAGESIZE);
    uint64_t page_mask = (page_size - 1);
    uint64_t mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);

    std::unique_lock<std::mutex> lck(mmap_mutex_);
    if (g_mmap_fd_tmp == 0) {
        g_mmap_fd_tmp = open("/dev/mem", O_RDWR | O_SYNC);
    }

    mmap_addr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mmap_fd_tmp, phys_addr & ~page_mask);
    if (mmap_addr != (void*)(-1))
        virt_addr = (void*)((char*)mmap_addr + (phys_addr & page_mask));
    else
        printf("**** sys_mmap failed\n");

    return virt_addr;
}

int user_sys_munmap(uint64_t phys_addr, void *virt_addr, size_t size)
{
    std::unique_lock<std::mutex> lck(mmap_mutex_);
    if (g_mmap_fd_tmp == 0) {
        return -1;
    }
    uint32_t page_size = sysconf(_SC_PAGESIZE);
    uint64_t page_mask = page_size - 1;
    uint64_t mmap_size = ((size) + (phys_addr & page_mask) + page_mask) & ~(page_mask);
    void* mmap_addr = (void*)((char*)virt_addr - (phys_addr & page_mask));
    if( munmap(mmap_addr, mmap_size) < 0) {
        printf("**** munmap failed\n");
    }
    return 0;
}

void user_sys_flush_cache(void *start, size_t size) {
    // impl accoridng to shm type
}
void user_sys_invalidate_cache(void *start, size_t size) {
    // impl accoridng to shm type
}
