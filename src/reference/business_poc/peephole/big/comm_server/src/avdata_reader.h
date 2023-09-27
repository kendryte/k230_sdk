#ifndef _AVDATA_READER_H__
#define _AVDATA_READER_H__

#include <unistd.h>
#include <string>
#include <memory>
#include "comm_server.h"

class AVDataReader {
  public:
    AVDataReader();
    ~AVDataReader();

    int Init(uint64_t &data_fifo_phy_addr, uint64_t &data_phy_addr, uint64_t &mem_size,
            IServerCallback *callback,  size_t mem_buf_size = 0);
    void DeInit();
  
  private:
    AVDataReader(const AVDataReader &) = delete;
    AVDataReader& operator=(const AVDataReader &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _EVENT_READER_H__
