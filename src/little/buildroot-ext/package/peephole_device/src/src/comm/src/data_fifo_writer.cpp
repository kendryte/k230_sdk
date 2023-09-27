#include <iostream>
#include <map>
#include <cstring>
#include <mutex>
#include "data_fifo_writer.h"
#include "k_datafifo.h"

static std::mutex s_map_mutex;
static std::map<uint64_t, IDataFifoRelease *> s_map;

struct DataFifoBlock {
    uint64_t phy_addr;
    int32_t channel;
    uint8_t data[0];
}__attribute__((packed));

static void datafifo_release(void *opaque) {
    DataFifoBlock *block = (DataFifoBlock*)opaque;
    IDataFifoRelease *callback = nullptr;
    std::unique_lock<std::mutex> lck(s_map_mutex);
    if (s_map.count(block->phy_addr)) {
        callback = s_map[block->phy_addr];
    }
    lck.unlock();

    // std::cout << "datafifo_release() called, phy_addr = " << block->phy_addr << std::endl;
    if (callback) {
        callback->release(block->data);
    }
}

class UserDataFifoWriter::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(int item_size, int item_count, uint64_t phy_addr, IDataFifoRelease *func) {
        std::unique_lock<std::mutex> lck(mutex_);
        item_size_block_ = (item_size + sizeof(DataFifoBlock) + 31)/ 32 * 32;
        // std::cout << "sizeof(DataFifoBlock) = " << sizeof(DataFifoBlock) << std::endl;
        k_datafifo_params_s datafifo_params;
        memset(&datafifo_params, 0, sizeof(datafifo_params));
        datafifo_params.u32EntriesNum = item_count;
        datafifo_params.u32CacheLineSize = item_size_block_;
        datafifo_params.bDataReleaseByWriter = K_TRUE;
        datafifo_params.enOpenMode = DATAFIFO_WRITER;
        k_s32 s32Ret = kd_datafifo_open_by_addr(&handle_, &datafifo_params, phy_addr);
        if (K_SUCCESS != s32Ret) {
            printf("%s open datafifo error:%x\n",__FUNCTION__,s32Ret);
            return -1;
        }
        
        s32Ret = kd_datafifo_cmd(handle_, DATAFIFO_CMD_SET_DATA_RELEASE_CALLBACK, (void*)&datafifo_release);
        if (K_SUCCESS != s32Ret) {
            printf("%s set release func callback error:%x\n", __FUNCTION__,s32Ret);
            return -1;
        }

        item_size_ = item_size;
        item_count_ = item_count;
        phy_addr_ = phy_addr;
        std::unique_lock<std::mutex> map_lck(s_map_mutex);
        s_map[phy_addr] = func;
        return 0;
    }

    void DeInit() {
        std::unique_lock<std::mutex> lck(mutex_);
        if (handle_) {
            kd_datafifo_write(handle_, NULL);
            kd_datafifo_close(handle_);
            handle_ = 0;
        }

        if (phy_addr_) {
            std::unique_lock<std::mutex> map_lck(s_map_mutex);
            auto iter = s_map.find(phy_addr_);
            if (iter != s_map.end()) s_map.erase(iter);
            phy_addr_ = 0;
        }
    }

    int Flush() {
	    std::unique_lock<std::mutex> lck(mutex_);
        if (!handle_) return -1;

        k_s32 s32Ret = K_FAILED;
        s32Ret = kd_datafifo_write(handle_, NULL);
        if (K_SUCCESS != s32Ret) {
            printf("%s write error:%x\n", __FUNCTION__,s32Ret);
            return -1;
        }
        return 0;
    }

    int Write(void *data, int channel) {
        std::unique_lock<std::mutex> lck(mutex_);
        if (!handle_) return -1;
        kd_datafifo_write(handle_, NULL);

        k_s32 s32Ret = K_FAILED;
        k_u32 availWriteLen = 0;
        s32Ret = kd_datafifo_cmd(handle_, DATAFIFO_CMD_GET_AVAIL_WRITE_LEN, &availWriteLen);
        if (K_SUCCESS != s32Ret) {
            printf("%s get available write len error:%x\n", __FUNCTION__,s32Ret);
            return -1;
        }

        if (availWriteLen >= item_size_block_) {
            DataFifoBlock *block = (DataFifoBlock*)tmp_write_buf_;
            // std::cout << "UserDataFifoWriter::Impl::Write phy_addr_ = " << phy_addr_ << std::endl;
            block->phy_addr = phy_addr_;
            block->channel = channel;
            memcpy(block->data, data, item_size_);
            s32Ret = kd_datafifo_write(handle_, block);
            if (K_SUCCESS != s32Ret) {
                printf("%s write error:%x\n", __FUNCTION__,s32Ret);
                return -1;
            }
            s32Ret = kd_datafifo_cmd(handle_, DATAFIFO_CMD_WRITE_DONE, NULL);
            if (K_SUCCESS != s32Ret) {
                printf("%s write done error:%x\n", __FUNCTION__,s32Ret);
                return -1;
            }
        } else {
            printf("%s no availWriteLen %d_%d\n",__FUNCTION__, availWriteLen, item_size_block_);
            return 1;
        }
        // std::cout << "UserDataFifoWriter::Impl::Write done" << std::endl;
        return 0;
    }
  
  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    int item_size_{0};
    int item_size_block_{0};
    int item_count_{0};
    uint64_t phy_addr_{0};
    std::mutex mutex_;
    k_datafifo_handle handle_{0};
    uint8_t tmp_write_buf_[1024]; // FIXME, 1024 is big enough
};

UserDataFifoWriter::UserDataFifoWriter() : impl_(std::make_unique<Impl>()) {}
UserDataFifoWriter::~UserDataFifoWriter() {}

int UserDataFifoWriter::Init(int item_size, int item_count, uint64_t phy_addr, IDataFifoRelease *func) {
    return impl_->Init(item_size, item_count, phy_addr, func);
}

void UserDataFifoWriter::DeInit() {
    impl_->DeInit();
}

int UserDataFifoWriter::Write(void *data, int channel) {
    return impl_->Write(data, channel);
}

int UserDataFifoWriter::Flush() {
    return impl_->Flush();
}
