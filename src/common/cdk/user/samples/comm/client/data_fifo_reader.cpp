#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cstring>
#include "data_fifo_reader.h"
#include "k_datafifo.h"

using namespace std::chrono_literals;

struct DataFifoBlock {
    uint64_t phy_addr;
    int32_t channel;
    uint8_t data[0];
}__attribute__((packed));

class UserDataFifoReader::Impl {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(int item_size, int item_count, uint64_t phy_addr, IDataFifoReadData *func) {
        std::unique_lock<std::mutex> lck(mutex_);
        size_t item_size_block = (item_size + sizeof(DataFifoBlock) + 31)/ 32 * 32;
        k_datafifo_params_s datafifo_params;
        memset(&datafifo_params, 0, sizeof(datafifo_params));
        datafifo_params.u32EntriesNum = item_count;
        datafifo_params.u32CacheLineSize = item_size_block;
        datafifo_params.bDataReleaseByWriter = K_TRUE;
        datafifo_params.enOpenMode = DATAFIFO_READER;
        k_s32 s32Ret = kd_datafifo_open_by_addr(&handle_, &datafifo_params, phy_addr);
        if (K_SUCCESS != s32Ret) {
            printf("%s open datafifo error:%x\n",__FUNCTION__,s32Ret);
            return -1;
        }
        item_size_ = item_size;
        item_count_ = item_count;
        item_size_block_ = item_size_block;
        callback_ = func;

        exit_flag_ = false;
        lck.unlock();

        thread_ = std::thread([this]() {
            while (!exit_flag_) {
                std::unique_lock<std::mutex> lck(mutex_);
                if (!handle_) {
                    break;
                }
                k_u32 readLen = 0;
                k_s32 s32Ret = kd_datafifo_cmd(handle_, DATAFIFO_CMD_GET_AVAIL_READ_LEN, &readLen);
                if (K_SUCCESS != s32Ret) {
                    printf("get available read len error:%x\n", s32Ret);
                    break;
                }
                while (readLen >= item_size_block_){
                    k_char* pBuf = nullptr;
                    s32Ret = kd_datafifo_read(handle_, (void**)&pBuf);
                    if (K_SUCCESS != s32Ret) {
                        printf("read error:%x\n", s32Ret);
                        exit_flag_.store(true);
                        break;
                    }
                    if (callback_) {
                        DataFifoBlock *blk = (DataFifoBlock*)pBuf;
                        // std::cout << "UserDataFifoReader::Impl::Init() -- read callback, phy_addr = " << blk->phy_addr << std::endl;
                        if (blk->phy_addr) {
                            callback_->on_read_data(blk->channel, blk->data);
                        }
                    }

                    s32Ret = kd_datafifo_cmd(handle_, DATAFIFO_CMD_READ_DONE, pBuf);
                    if (K_SUCCESS != s32Ret) {
                        printf("read done error:%x\n", s32Ret);
                        exit_flag_.store(true);
                        break;
                    }
                    readLen -= item_size_block_;
                }
                lck.unlock();
                std::this_thread::sleep_for(10ms);
                lck.lock();
            }
        });
        return 0;
    }

    void DeInit() {
        exit_flag_.store(true);
        if (thread_.joinable()) {
            thread_.join();
        }
        std::unique_lock<std::mutex> lck(mutex_);
        if (handle_) {
            kd_datafifo_close(handle_);
            handle_ = 0;
        }
    }


  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    int item_size_{0};
    int item_count_{0};
    size_t item_size_block_{0};
    IDataFifoReadData *callback_{nullptr};
    std::mutex mutex_;
    k_datafifo_handle handle_{0};
    std::thread thread_;
    std::atomic<bool> exit_flag_{false};
    
};

UserDataFifoReader::UserDataFifoReader() : impl_(std::make_unique<Impl>()) {}
UserDataFifoReader::~UserDataFifoReader() {}

int UserDataFifoReader::Init(int item_size, int item_count, uint64_t phy_addr, IDataFifoReadData *func) {
    return impl_->Init(item_size, item_count, phy_addr, func);
}

void UserDataFifoReader::DeInit() {
    impl_->DeInit();
}
