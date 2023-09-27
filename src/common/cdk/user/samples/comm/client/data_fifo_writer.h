#ifndef _USER_DATAFIFO_WRITER_H__
#define _USER_DATAFIFO_WRITER_H__

#include <unistd.h>
#include <string>
#include <memory>

class IDataFifoRelease {
  public:
    virtual ~IDataFifoRelease() {}
    virtual void release(void *opaque) = 0;
};

class UserDataFifoWriter {
  public:
    UserDataFifoWriter();
    ~UserDataFifoWriter();

    int Init(int item_size, int item_count, uint64_t phy_addr, IDataFifoRelease *func);
    int Write(void *data, int channel = 0);
    void DeInit();
    int Flush();
  
  private:
    UserDataFifoWriter(const UserDataFifoWriter &) = delete;
    UserDataFifoWriter& operator=(const UserDataFifoWriter &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _USER_DATAFIFO_WRITER_H__

