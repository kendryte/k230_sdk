#include <iostream>
#include <mutex>
#include "comm_server.h"
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

    int Init(uint64_t &data_fifo_phy_addr, uint64_t &data_phy_addr, uint64_t &mem_size,
            IServerCallback *callback,  size_t mem_buf_size) {
        std::unique_lock<std::mutex> lck(mutex_);

        if (df_reader_.Init(sizeof(AVDataBlock), 32, data_fifo_phy_addr, this) < 0) {
            return -1;
        }
        df_phy_addr_ = data_fifo_phy_addr;

        if (shm_.Init(data_phy_addr, virt_addr_, mem_size, mem_buf_size) < 0) {
            std::cout << "AVDataReader::Impl::Init() shm_init failed" << std::endl;
            return -1;
        }
        callback_ = callback;
        return 0;
    }

    void DeInit() {
        std::unique_lock<std::mutex> lck(mutex_);
        if (virt_addr_) {
            shm_.DeInit();
            virt_addr_ = nullptr;
        }
        if (df_phy_addr_) {
            df_reader_.DeInit();
            df_phy_addr_ = 0;
        }
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
        data.type = static_cast<AVEncFrameData::Type>(blk->codec_type);
        data.sequence = blk->sequence;
        data.flags = blk->flags;
        shm_.InvalidateCache(data.data, data.size);
        switch(data.type) {
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
    uint64_t df_phy_addr_{0};
    MyDataShm shm_;
    void *virt_addr_{nullptr};
    IServerCallback *callback_{nullptr};
};

AVDataReader::AVDataReader() : impl_(std::make_unique<Impl>()) {}
AVDataReader::~AVDataReader() {}

int AVDataReader::Init(uint64_t &data_fifo_phy_addr, uint64_t &data_phy_addr, uint64_t &mem_size,
            IServerCallback *callback,  size_t mem_buf_size) {
    return impl_->Init(data_fifo_phy_addr, data_phy_addr, mem_size, callback, mem_buf_size);
}

void AVDataReader::DeInit() {
    impl_->DeInit();
}

