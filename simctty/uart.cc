// simctty
// Copyright 2014 Tom Harwood

#include "simctty/uart.h"

#include <stdarg.h>

UART::UART()
  :
    BusDevice(),
    ier_(0),
    lcr_(3),
    mcr_(0),
    dll_(0),
    dlm_(0),
    transmit_ready_interrupt_(false),
    pretend_enable_fifos_(false) {
}

UART::~UART() {
}

uint8 UART::Load8(uint32 address, Exception* exception) const {
  address -= kMinUartAddress;
  *exception = kExceptionNone;

  if (lcr_ & 0x80) {
    switch (address) {
    case 0:
      return dll_;
    case 1:
      return dlm_;
    }
  }

  uint32 value;
  switch (address) {
  case 0:
    if (keypress_fifo_.empty()) {
      return 0;
    } else {
      value = keypress_fifo_.front();
      keypress_fifo_.pop_front();
      return value;
    }
    break;
  case 1:
    return ier_;
  case 2:
    value = (pretend_enable_fifos_ ? 0xc0 : 0x00) |
      (IsInterruptAsserted() ? 0x0 : 0x1) |
      (IsDataReadyInterrupt() ? 0x4 :
       IsTransmitReadyInterrupt() ? 0x2 : 0);
    transmit_ready_interrupt_ = false;
    return value;
  case 3:
    return lcr_;
  case 4:
    return mcr_;
  case 5:
    // Line Status register.
    return
      0x60 |                           // Can always transmit.
      (keypress_fifo_.empty() ? 0x0 : 0x1);  // Data available to read.
  case 6:
    return 0;
  }

  return 0;
}

void UART::Store8(uint32 address, uint8 value, Exception* exception) {
  address -= kMinUartAddress;
  *exception = kExceptionNone;

  if (lcr_ & 0x80) {
    switch (address) {
    case 0:
      dll_ = value;
      return;
    case 1:
      dlm_ = value;
      return;
    }
  }

  switch (address) {
  case 0:
    display_fifo_.push_back(value);
    if (ier_ & 0x2) {
      transmit_ready_interrupt_ = true;
    } else {
      transmit_ready_interrupt_ = false;
    }
    break;
  case 1:
    ier_ = value;
    break;
  case 2:
    pretend_enable_fifos_ = value & 0x1;
    break;
  case 3:
    lcr_ = value;
    break;
  case 4:
    mcr_ = value;
    break;
  default:
    break;
  }
}

bool UART::IsInterruptAsserted() const {
  // Interrupt enabled?
  if (IsDataReadyInterrupt() || IsTransmitReadyInterrupt()) {
    return true;
  }

  return false;
}

bool UART::IsDataReadyInterrupt() const {
  return !keypress_fifo_.empty() && ier_ & 0x1;
}

bool UART::IsTransmitReadyInterrupt() const {
  return ier_ & 0x2 && transmit_ready_interrupt_;
}

void UART::Keypress(uint8 c) {
  keypress_fifo_.push_back(c);
}

uint8 UART::Read() {
  if (display_fifo_.empty()) {
    return 0;
  }

  uint8 key = display_fifo_.front();
  display_fifo_.pop_front();

  return key;
}

bool UART::CanRead() const {
  return !display_fifo_.empty();
}

