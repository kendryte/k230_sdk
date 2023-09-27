#ifndef MY_DATA_SHM_H_
#define MY_DATA_SHM_H_

#include <iostream>
#include <mutex>
#include "mpi_vb_api.h"
#include "mpi_sys_api.h"

class MyDataShm {
  public:
    MyDataShm() {}
    ~MyDataShm() { DeInit(); }

    int Init(uint64_t &phy_addr, void *&virt_addr, uint64_t &mem_size, size_t mem_buf_size = 0) {
        if (mem_buf_size) {
            vb_pool_size_ = (mem_buf_size + 4095) / 4096 * 4096;
        }
        k_vb_pool_config pool_config;
        memset(&pool_config, 0, sizeof(pool_config));
        pool_config.blk_cnt = 1;
        pool_config.blk_size = vb_pool_size_;
        pool_config.mode = VB_REMAP_MODE_NOCACHE;
        vb_pool_id_ = kd_mpi_vb_create_pool(&pool_config);
        if (vb_pool_id_ == VB_INVALID_POOLID) {
            std::cout << "MyDataShm::Init() kd_mpi_vb_create_pool failed" << std::endl;
            return -1;
        }
        vb_blk_handle_ = kd_mpi_vb_get_block(vb_pool_id_, vb_pool_size_, NULL);
        if (vb_blk_handle_ == VB_INVALID_HANDLE) {
            std::cout << "MyDataShm::Init() kd_mpi_vb_get_block failed" << std::endl;
            return -1;
        }
        phy_addr_ = kd_mpi_vb_handle_to_phyaddr(vb_blk_handle_);
        virt_addr_ = kd_mpi_sys_mmap(phy_addr_, vb_pool_size_);
        if(!virt_addr_) {
            std::cout << "MyDataShm::Init() kd_mpi_sys_mmap failed" << std::endl;
            return -1;
        }
        phy_addr = phy_addr_;
        virt_addr = virt_addr_;
        mem_size = vb_pool_size_;
        return 0;
    }

    void DeInit() {
        if (vb_pool_id_ != VB_INVALID_POOLID) {
            if (vb_blk_handle_ != VB_INVALID_HANDLE) {
                if (virt_addr_) kd_mpi_sys_munmap(virt_addr_, vb_pool_size_), virt_addr_ = nullptr;
                kd_mpi_vb_release_block(vb_blk_handle_);
                vb_blk_handle_ = VB_INVALID_HANDLE;
                phy_addr_ = 0;
            }
            kd_mpi_vb_destory_pool(vb_pool_id_);
            vb_pool_id_ = VB_INVALID_POOLID;
        }
    }

    void FlushCache(void *start, size_t size) {}
    void InvalidateCache(void *start, size_t size) {}

  private:
    MyDataShm(const MyDataShm &) = delete;
    MyDataShm& operator=(const MyDataShm &) = delete;

  private:
    k_u32 vb_pool_id_{VB_INVALID_POOLID};
    k_vb_blk_handle vb_blk_handle_{VB_INVALID_HANDLE};
    uint64_t phy_addr_{0};
    void *virt_addr_{nullptr};
    size_t vb_pool_size_ = 2 * 1024 * 1024;
};

#endif // MY_DATA_SHM_H_
