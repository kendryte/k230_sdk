#include <iostream>
#include <mutex>
#include "event_writer.h"
#include "data_fifo_writer.h"
#include "data_shm.h"

struct EventDataBlock {
    int32_t type{-1};
    int32_t len{0};
    int32_t offset{0};
    int32_t jpeg_size{0};
    int64_t timestamp_ms{0};
}__attribute__((packed));

class EventWriter::Impl : public IDataFifoRelease {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(uint64_t &data_fifo_phy_addr, uint64_t &data_phy_addr, uint64_t &mem_size, size_t mem_buf_size) {
        std::unique_lock<std::mutex> lck(mutex_);
        if (df_writer_.Init(sizeof(EventDataBlock), 32, data_fifo_phy_addr, this) < 0) {
            return -1;
        }
        df_phy_addr_ = data_fifo_phy_addr;

         if (shm_.Init(data_phy_addr, virt_addr_, shm_size_, mem_buf_size) < 0) {
            std::cout << "EventWriter::Impl::Init() shm_init failed" << std::endl;
            return -1;
        }
        mem_size = shm_size_;
        //
        write_idx_ = 0;
        read_idx_ = 0;
        avail_len_ = shm_size_;
        return 0;
    }

    void DeInit() {
        std::unique_lock<std::mutex> lck(mutex_);
        if (df_phy_addr_) {
            df_writer_.DeInit();
            df_phy_addr_ = 0;
        }
        if (virt_addr_) {
            shm_.DeInit();
            virt_addr_ = nullptr;
        }
    }

    int Write(const UserEventData &event, IInterruptCallback *callback) {
        std::unique_lock<std::mutex> lck(mutex_);
        size_t offset = 0;
        size_t discard_len = 0;
        size_t size = 0;
        size_t write_idx = write_idx_;
        if (event.jpeg && event.jpeg_size) {
            size = (event.jpeg_size + 7) / 8 * 8;
            while (size > get_avail_len()) {
                if (callback && callback->Exit()) return -2;
                std::cout << "EventWriter::Impl::Write() --1 retry" << std::endl;
                df_writer_.Flush();
                usleep(1000 * 10);
            }

            // always make the data continous
            if (write_idx + size > shm_size_) {
                while (get_avail_len() < shm_size_ - write_idx + size) {
                    if (callback && callback->Exit()) return -2;
                    std::cout << "EventWriter::Impl::Write() --2 retry" << std::endl;
                    df_writer_.Flush();
                    usleep(1000 * 10);
                }
                discard_len = shm_size_ - write_idx;
                uint8_t *buf = (uint8_t *)virt_addr_;
                offset = 0;
                memcpy(&buf[offset], event.jpeg, event.jpeg_size);
                write_idx = size;
            } else {
                offset = write_idx;
                uint8_t *buf = (uint8_t *)virt_addr_;
                memcpy(&buf[offset], event.jpeg, event.jpeg_size);
                write_idx += size;
                if(write_idx == shm_size_) write_idx = 0;
            }
            // std::cout << "EventWriter::Impl::Write() called, next write_idx = " << write_idx_ << std::endl;
        }
        EventDataBlock block;
        block.type = static_cast<int>(event.type);
        block.len = sizeof(EventDataBlock) - 2 * sizeof(int);
        block.offset = offset;
        block.jpeg_size = (event.jpeg && event.jpeg_size) ? event.jpeg_size : 0;
        block.timestamp_ms = event.timestamp_ms;
        // std::cout << "EventWriter::Impl::Write() do write, type = " << block.type << std::endl;
        shm_.FlushCache((uint8_t*)virt_addr_ + offset, block.jpeg_size);

        dec_avail_len(discard_len + size);
        int ret;
        while ((ret = df_writer_.Write(&block)) > 0) {
            if (callback && callback->Exit()) break;
            usleep(1000);
        }
        if (ret) {
            inc_avail_len(discard_len + size);
            return -1;
        }
        if (event.jpeg && event.jpeg_size) {
            write_idx_ = write_idx;
        }
        return 0;
    }

    // IDataFifoRelease
    virtual void release(void *opaque) override {
        EventDataBlock *block = (EventDataBlock*)opaque;
        size_t size = (block->jpeg_size + 7)/ 8 * 8;
        if (read_idx_ + size > shm_size_) {
            inc_avail_len(shm_size_ - read_idx_);
            read_idx_ = 0;
        }
        read_idx_ += size;
        if (read_idx_ == shm_size_) read_idx_ = 0;
        inc_avail_len(size);
    }

  private:
    void inc_avail_len(size_t size) { avail_len_ += size; }
    void dec_avail_len(size_t size) { avail_len_ -= size; }
    size_t get_avail_len() { return avail_len_;}

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    std::mutex mutex_;
    UserDataFifoWriter df_writer_;
    uint64_t df_phy_addr_{0};
    MyDataShm shm_;
    void *virt_addr_{nullptr};
    size_t shm_size_{0};
    size_t write_idx_ = 0;
    size_t read_idx_ = 0;
    size_t avail_len_ = 0;
    
};

EventWriter::EventWriter() : impl_(std::make_unique<Impl>()) {}
EventWriter::~EventWriter() {}

int EventWriter::Init(uint64_t &data_fifo_phy_addr, uint64_t &data_phy_addr, uint64_t &mem_size, size_t mem_buf_size) {
    return impl_->Init(data_fifo_phy_addr, data_phy_addr, mem_size, mem_buf_size);
}

void EventWriter::DeInit() {
    impl_->DeInit();
}

int EventWriter::Write(const UserEventData &event, IInterruptCallback *callback){
    return impl_->Write(event, callback);
}
