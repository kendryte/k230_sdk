#include <iostream>
#include <mutex>
#include "avdata_reader.h"
#include "data_fifo_reader.h"
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

class AVDataReader::Impl : public IDataFifoReadData {
  public: 
    Impl() {}
    ~Impl() { DeInit(); }

    int Init(uint64_t data_fifo_phy_addr, uint64_t data_phy_addr, uint64_t mem_size, IClientCallback *callback) {
        std::unique_lock<std::mutex> lck(mutex_);
        data_fifo_phy_addr_ = data_fifo_phy_addr;
        data_phy_addr_ = data_phy_addr;
        mem_size_ = mem_size;
        callback_ = callback;

        virt_addr_ = user_sys_mmap(data_phy_addr_, mem_size_);
        if(!virt_addr_) {
            std::cout << "AVDataReader::Impl::Init() user_sys_mmap failed" << std::endl;
            return -1;
        }

        return df_reader_.Init(sizeof(AVDataBlock), 32, data_fifo_phy_addr_, this);
    }

    void DeInit() {
        std::unique_lock<std::mutex> lck(mutex_);
        if (virt_addr_) user_sys_munmap(data_phy_addr_, virt_addr_, mem_size_), virt_addr_ = nullptr;
        df_reader_.DeInit();
    }

    virtual void on_read_data(int channel, void *opaque) override {
        std::unique_lock<std::mutex> lck(mutex_);
        if (!virt_addr_) return;
        if (!callback_) return;
        AVDataBlock *blk = (AVDataBlock*)opaque;
        if ( blk->codec_type <= static_cast<int>(AVEncFrameData::Type::INVALID)
           || blk->codec_type >= static_cast<int>(AVEncFrameData::Type::BOTTOM)) {
            std::cout << "AVDataReader::Impl::on_read_data()  invalid codec_type : " << blk->codec_type << std::endl;
            return;
        }
        AVEncFrameData data;
        data.data = (uint8_t*)virt_addr_ + blk->offset;
        data.size = blk->size;
        data.keyframe = blk->is_key_frame ?  true : false;
        data.timestamp_ms = blk->timestamp_ms;
        data.sequence = blk->sequence;
        data.flags = blk->flags;
        data.type = static_cast<AVEncFrameData::Type>(blk->codec_type);
        user_sys_invalidate_cache(data.data, data.size);
        switch(data.type) {
        case AVEncFrameData::Type::H264:
        case AVEncFrameData::Type::H265: {
            if(callback_) callback_->OnVEncFrameData(data);
            break;
        }
        case AVEncFrameData::Type::PCMU: {
            if(callback_) callback_->OnAEncFrameData(data);
            break;
        }
        default: {
            std::cout << "AVDataReader::Impl::on_read_data()  codec_type not supported yet" << std::endl;
            break;
        }
        }
    }

  private:
    Impl(const Impl &) = delete;
    Impl& operator=(const Impl &) = delete;

  private:
    std::mutex mutex_;
    UserDataFifoReader df_reader_;
    uint64_t data_fifo_phy_addr_{0};
    uint64_t data_phy_addr_{0};
    size_t mem_size_{0};
    IClientCallback *callback_{nullptr};
    void *virt_addr_{nullptr};    
};

AVDataReader::AVDataReader() : impl_(std::make_unique<Impl>()) {}
AVDataReader::~AVDataReader() {}

int AVDataReader::Init(uint64_t data_fifo_phy_addr, uint64_t data_phy_addr, uint64_t mem_size, IClientCallback *callback) {
    return impl_->Init(data_fifo_phy_addr, data_phy_addr, mem_size, callback);
}

void AVDataReader::DeInit() {
    impl_->DeInit();
}

