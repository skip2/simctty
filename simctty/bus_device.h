// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_BUS_DEVICE_H_
#define SIMCTTY_BUS_DEVICE_H_

#include "simctty/exception.h"
#include "simctty/types.h"

class BusDevice {
 public:
  BusDevice() {}
  virtual ~BusDevice() {}

  virtual uint8 Load8(uint32 address, Exception* exception) const {
    *exception = kExceptionBusError;
    return 0;
  }

  virtual uint16 Load16(uint32 address, Exception* exception) const {
    *exception = kExceptionBusError;
    return 0;
  }

  virtual uint32 Load32(uint32 address, Exception* exception) const {
    *exception = kExceptionBusError;
    return 0;
  }

  virtual void Store8(uint32 address, uint8 value, Exception* exception) {
    *exception = kExceptionBusError;
  }

  virtual void Store16(uint32 address, uint16 value, Exception* exception) {
    *exception = kExceptionBusError;
  }

  virtual void Store32(uint32 address, uint32 value, Exception* exception) {
    *exception = kExceptionBusError;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(BusDevice);
};

#endif  // SIMCTTY_BUS_DEVICE_H_

