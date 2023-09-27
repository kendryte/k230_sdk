#ifndef _EVENT_WRITER_H__
#define _EVENT_WRITER_H__

#include <unistd.h>
#include <string>
#include <memory>
#include "comm_server.h"

class EventWriter {
  public:
    EventWriter();
    ~EventWriter();

    int Init(uint64_t &data_fifo_phy_addr, uint64_t &data_phy_addr, uint64_t &mem_size, size_t mem_buf_size = 0);
    void DeInit();
    int Write(const UserEventData &event, IInterruptCallback *callback);
  
  private:
    EventWriter(const EventWriter &) = delete;
    EventWriter& operator=(const EventWriter &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _EVENT_WRITER_H__

