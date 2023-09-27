#ifndef _AVDATA_WRITER_H__
#define _AVDATA_WRITER_H__

#include <unistd.h>
#include <string>
#include <memory>
#include "comm_client.h"

class AVDataWriter {
  public:
    AVDataWriter();
    ~AVDataWriter();

    int Init(uint64_t data_fifo_phy_addr, uint64_t data_phy_addr, uint64_t mem_size);
    void DeInit();
    int Write(const AVEncFrameData &data, IInterruptCallback *callback);
  
  private:
    AVDataWriter(const AVDataWriter &) = delete;
    AVDataWriter& operator=(const AVDataWriter &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _AVDATA_WRITER_H__

