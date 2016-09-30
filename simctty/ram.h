// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_RAM_H_
#define SIMCTTY_RAM_H_

#include "simctty/bus_device.h"
#include "simctty/exception.h"
#include "simctty/types.h"

const static uint32 kMaxRamAddress = 0x2000000 - 1;

class RAM : public BusDevice {
 public:
  RAM();
  ~RAM();

  uint32 Size() const;
  uint8* Raw();

  bool LoadImage(const uint8* data, size_t len, size_t offset = 0);

  virtual uint8 Load8(uint32 address, Exception* exception) const;
  virtual void Store8(uint32 address, uint8 value, Exception* exception);

  virtual uint16 Load16(uint32 address, Exception* exception) const;
  virtual void Store16(uint32 address, uint16 value, Exception* exception);

  virtual uint32 Load32(uint32 address, Exception* exception) const;
  virtual void Store32(uint32 address, uint32 value, Exception* exception);

  void DumpU8(const char* filename="dump8") const;

 private:
  const size_t size_;
  uint8* ram_;

  DISALLOW_COPY_AND_ASSIGN(RAM);
};

#endif  // SIMCTTY_RAM_H_

