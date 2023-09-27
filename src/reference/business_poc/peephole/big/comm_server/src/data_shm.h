#ifndef MY_DATA_SHM_H_
#define MY_DATA_SHM_H_

#include <iostream>
#include <string>
#include "mpi_sys_api.h"

class MyDataShm {
  public:
    MyDataShm() {}
    ~MyDataShm() { DeInit(); }

    int Init(uint64_t &phy_addr, void *&virt_addr, uint64_t &mem_size, size_t mem_buf_size = 0) {
        if (mem_buf_size) {
            shm_size_ = (mem_buf_size + 4095) / 4096 * 4096;
        }

        std::string name = "shm_" + std::to_string((long)this);
        if (kd_mpi_sys_mmz_alloc(&phy_addr_, &virt_addr_, name.c_str(), "anonymous", shm_size_)) {
            std::cout << "MyDataShm::Init() -- kd_mpi_sys_mmz_alloc failed" << std::endl;
            return -1;
        }

        phy_addr = phy_addr_;
        virt_addr = virt_addr_;
        mem_size = shm_size_;

        std::cout << "MyDataShm::Init() -- kd_mpi_sys_mmz_alloc Done, name : " << name << std::endl;
        return 0;
    }

    void DeInit() {
        if (phy_addr_ && virt_addr_) {
            kd_mpi_sys_mmz_free(phy_addr_, virt_addr_);
            phy_addr_ = 0;
            virt_addr_ = nullptr;
        }
    }

    void FlushCache(void *start, size_t size) {}
    void InvalidateCache(void *start, size_t size) {}

  private:
    MyDataShm(const MyDataShm &) = delete;
    MyDataShm& operator=(const MyDataShm &) = delete;

  private:
    uint64_t phy_addr_{0};
    void *virt_addr_{nullptr};
    size_t shm_size_ = 2 * 1024 * 1024;
};

#endif // MY_DATA_SHM_H_
