#ifndef _USER_DATAFIFO_READER_H__
#define _USER_DATAFIFO_READER_H__

#include <unistd.h>
#include <string>
#include <memory>

class IDataFifoReadData {
  public:
    virtual ~IDataFifoReadData() {}
    virtual void on_read_data(int channel, void *opaque) = 0;
};

class UserDataFifoReader {
  public:
    UserDataFifoReader();
    ~UserDataFifoReader();

    int Init(int item_size, int item_count, uint64_t &phy_addr, IDataFifoReadData *func);
    void DeInit();
  
  private:
    UserDataFifoReader(const UserDataFifoReader &) = delete;
    UserDataFifoReader& operator=(const UserDataFifoReader &) = delete;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_{nullptr};
};

#endif // _USER_DATAFIFO_READER_H__

