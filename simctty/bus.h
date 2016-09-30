// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_BUS_H_
#define SIMCTTY_BUS_H_

#include "simctty/bus_device.h"
#include "simctty/exception.h"
#include "simctty/ram.h"
#include "simctty/types.h"
#include "simctty/uart.h"

class CPU;
class Bus {
 public:
  Bus();
  ~Bus();

  RAM* GetRAM();
  const RAM* GetRAM() const;

  BusDevice* GetDevice(uint32 address);
  const BusDevice* GetDevice(uint32 address) const;

  UART* GetUART();

  uint32 Interrupts() const;

 private:
  UART uart_;
  RAM ram_;

  CPU* cpu_;

  DISALLOW_COPY_AND_ASSIGN(Bus);
};

#endif  // SIMCTTY_BUS_H_

