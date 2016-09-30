// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_UART_H_
#define SIMCTTY_UART_H_

#include <list>

#include "simctty/bus_device.h"
#include "simctty/exception.h"
#include "simctty/types.h"

using std::list;

const static uint32 kMinUartAddress = 0x90000000;
const static uint32 kMaxUartAddress = 0x90000100;

class UART : public BusDevice {
 public:
  UART();
  ~UART();

  virtual uint8 Load8(uint32 address, Exception* exception) const;
  virtual void Store8(uint32 address, uint8 value, Exception* exception);

  bool IsInterruptAsserted() const;

  void Keypress(uint8 c);
  uint8 Read();
  bool CanRead() const;

 private:
  mutable list<uint8> keypress_fifo_;
  list<uint8> display_fifo_;

  uint8 ier_;  // Interrupt enable register. R/W
  // IID is R only.
  // FCO is W only.

  uint8 lcr_;  // Line control register. R/W.
  uint8 mcr_;  // Modem control register. R/W.

  uint8 dll_;  // Divisor latch LSB. Echo only.
  uint8 dlm_;  // Divisor latch MSB. Echo only.

  const static size_t kIEReceiveData = 0;
  const static size_t kIETransmitRegEmpty = 1;
  const static size_t kIELineStatusChange = 2;
  const static size_t kIEModemStatusChange = 3;

  bool IsDataReadyInterrupt() const;
  bool IsTransmitReadyInterrupt() const;

  mutable bool transmit_ready_interrupt_;
  bool pretend_enable_fifos_;

  DISALLOW_COPY_AND_ASSIGN(UART);
};

#endif  // SIMCTTY_UART_H_


