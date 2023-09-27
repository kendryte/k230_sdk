#include <iostream>
#include <mutex>
#include "event_reader.h"
#include "data_fifo_reader.h"
#include "data_shm.h"

struct EventDataBlock {
    int32_t type{-1};
    int32_t len{0};
    int32_t offset{0};
    int32_t jpeg_size{0};
    int64_t timestamp_ms{0};
}__attribute__((packed));;

class EventReader::Impl : public IDataFifoReadData {
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
            std::cout << "EventReader::Impl::Init() user_sys_mmap failed" << std::endl;
            return -1;
        }

        return df_reader_.Init(sizeof(EventDataBlock), 32, data_fifo_phy_addr_, this);
    }

    void DeInit() {
        std::unique_lock<std::mutex> lck(mutex_);
        df_reader_.DeInit();
        if (virt_addr_) user_sys_munmap(data_phy_addr_, virt_addr_, mem_size_), virt_addr_ = nullptr;
    }

    virtual void on_read_data(int channel, void *opaque) override {
        if (!callback_) return;
        EventDataBlock *blk = (EventDataBlock*)opaque;
        if ( blk->type <= static_cast<int>(UserEventData::EventType::INVALID)
           || blk->type >= static_cast<int>(UserEventData::EventType::BOTTOM)) {
            std::cout << "EventReader::Impl::on_read_data()  invalid type : " << blk->type << std::endl;
            return;
        }
        uint8_t *base = (uint8_t*)virt_addr_;

        UserEventData event; 
        event.type = static_cast<UserEventData::EventType>(blk->type);
        event.timestamp_ms = blk->timestamp_ms;
        switch(event.type) {
        case UserEventData::EventType::PIR_WAKEUP:
        case UserEventData::EventType::KEY_WAKEUP:
        case UserEventData::EventType::STAY_ALARM: {
            event.jpeg_size = blk->jpeg_size;
            event.jpeg = base + blk->offset;
            break;
        }
        default: {
            std::cout << "EventReader::Impl::on_read_data()  type not supported yet" << std::endl;
            return;
        }
        }
        user_sys_invalidate_cache(event.jpeg, event.jpeg_size);
        if(callback_) callback_->OnEvent(event);
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

EventReader::EventReader() : impl_(std::make_unique<Impl>()) {}
EventReader::~EventReader() {}

int EventReader::Init(uint64_t data_fifo_phy_addr, uint64_t data_phy_addr, uint64_t mem_size, IClientCallback *callback) {
    return impl_->Init(data_fifo_phy_addr, data_phy_addr, mem_size, callback);
}

void EventReader::DeInit() {
    impl_->DeInit();
}

