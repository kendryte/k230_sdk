#include <iostream>
#include <mutex>
#include <cstring>
#include "avdata_writer.h"
#include "data_fifo_writer.h"
#include "data_shm.h"

struct AVDataBlock {
    int32_t type{-1};
    int32_t len{0};
    int32_t codec_type{0};
    int32_t is_key_frame{0};
    int32_t offset{0};
    int32_t size{0};
    int64_t timestamp_ms{0};
    uint32_t sequence{0};
    uint32_t flags{0};
} __attribute__((packed));

class AVDataWriter::Impl : public IDataFifoRelease {
  public:
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(uint64_t data_fifo_phy_addr, uint64_t data_phy_addr, uint64_t mem_size) {
        std::unique_lock<std::mutex> lck(mutex_);
        if (df_writer_.Init(sizeof(AVDataBlock), 32, data_fifo_phy_addr, this) < 0) {
            return -1;
        }
        df_phy_addr_ = data_fifo_phy_addr;
        shm_size_ = (size_t)(mem_size & 0xffffffff);
        phy_addr_ = data_phy_addr;
        
        virt_addr_ = user_sys_mmap(phy_addr_, shm_size_);
        if(!virt_addr_) {
            std::cout << "AVDataWriter::Impl::Init() user_sys_mmap failed" << std::endl;
            return -1;
        }
        //
        write_idx_ = 0;
        read_idx_  = 0;
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
            user_sys_munmap(phy_addr_, virt_addr_, shm_size_);
            virt_addr_ = nullptr;
        }
    }

    int Write(const AVEncFrameData &data, IInterruptCallback *callback) {
        std::unique_lock<std::mutex> lck(mutex_);
        if (!df_phy_addr_ || !virt_addr_) return -1;
        if (!data.data || !data.size) return  -1;
        if (data.type <=  AVEncFrameData::Type::INVALID || data.type >=  AVEncFrameData::Type::BOTTOM) return -1;

        size_t offset = 0;
        uint8_t *buf = (uint8_t *)virt_addr_;
        size_t size = (data.size + 7) / 8 * 8;
        while (size > get_avail_len() ) {
            if (callback && callback->Exit()) return -2;
            std::cout << "AVDataWriter::Impl::Write() --1 retry" << std::endl;
            df_writer_.Flush();
            usleep(1000 * 10);
        }
        size_t write_idx = write_idx_;
        size_t discard_len = 0;
        // always make the data continous
        if (write_idx + size > shm_size_) {
            while (get_avail_len() < shm_size_ - write_idx + size) {
                if (callback && callback->Exit()) return -2;
                std::cout << "AVDataWriter::Impl::Write() --2 retry" << std::endl;
                df_writer_.Flush();
                usleep(1000 * 10);
            }
            discard_len = shm_size_ - write_idx;
            uint8_t *buf = (uint8_t *)virt_addr_;
            offset = 0;
            memcpy(&buf[offset], data.data, data.size);
            write_idx = size;
        } else {
            offset = write_idx;
            uint8_t *buf = (uint8_t *)virt_addr_;
            memcpy(&buf[offset], data.data, data.size);
            write_idx += size;
            if(write_idx == shm_size_) write_idx = 0;
        }

        AVDataBlock block;
        block.type = 0; // reserved
        block.len = sizeof(AVDataBlock) - 2 * sizeof(int);
        block.offset = offset;
        block.size = data.size;
        block.timestamp_ms = data.timestamp_ms;
        block.codec_type = static_cast<int>(data.type);
        block.is_key_frame = data.keyframe ? 1 : 0;
        block.sequence = data.sequence;
        block.flags = data.flags;
        user_sys_flush_cache((uint8_t*)virt_addr_ + offset, block.size);

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
        write_idx_ = write_idx;
        return 0;
    }
  
    // IDataFifoRelease
    virtual void release(void *opaque) override {
        AVDataBlock *block = (AVDataBlock*)opaque;
        size_t size = (block->size + 7)/ 8 * 8;
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
    uint64_t phy_addr_{0};
    void *virt_addr_{nullptr};
    size_t shm_size_{0};
    size_t write_idx_ = 0;
    size_t read_idx_ = 0;
    size_t avail_len_ = 0;
};

AVDataWriter::AVDataWriter() : impl_(std::make_unique<Impl>()) {}
AVDataWriter::~AVDataWriter() {}

int AVDataWriter::Init(uint64_t data_fifo_phy_addr, uint64_t data_phy_addr, uint64_t mem_size) {
    return impl_->Init(data_fifo_phy_addr, data_phy_addr, mem_size);
}

void AVDataWriter::DeInit() {
    impl_->DeInit();
}

int AVDataWriter::Write(const AVEncFrameData &data, IInterruptCallback *callback){
    return impl_->Write(data, callback);
}
