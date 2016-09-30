// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_SYSTEM_H_
#define SIMCTTY_SYSTEM_H_

#include "simctty/bus.h"
#include "simctty/cpu.h"
#include "simctty/types.h"

class RAM;
class System {
 public:
  System();
  ~System();

  CPU* GetCPU();
  const RAM* GetRAM() const;
  RAM* GetRAM();
  Bus* GetBus();
  UART* GetUART();

  bool LoadImageFile(const char* filename, uint32 start_address);
  size_t LoadImage(const uint8* data, size_t length, uint32 start_address);
  bool Run(size_t cycles = 0);

 private:
  Bus bus_;
  CPU cpu_;

  DISALLOW_COPY_AND_ASSIGN(System);
};

#endif  // SIMCTTY_SYSTEM_H_

