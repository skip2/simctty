// simctty
// Copyright 2014 Tom Harwood

#include "simctty/bus.h"

Bus::Bus() {
}

Bus::~Bus() {
}

RAM* Bus::GetRAM() {
  return &ram_;
}

const RAM* Bus::GetRAM() const {
  return &ram_;
}

BusDevice* Bus::GetDevice(uint32 address) {
  BusDevice* device_;

  if (address <= kMaxRamAddress) {
    device_ = &ram_;
  } else if (address >= kMinUartAddress && address <= kMaxUartAddress) {
    device_ = &uart_;
  } else {
    device_ = nullptr;
  }

  return device_;
}

const BusDevice* Bus::GetDevice(uint32 address) const {
  const BusDevice* device_;

  if (address <= kMaxRamAddress) {
    device_ = &ram_;
  } else if (address >= kMinUartAddress && address <= kMaxUartAddress) {
    device_ = &uart_;
  } else {
    device_ = nullptr;
  }

  return device_;
}

uint32 Bus::Interrupts() const {
  return uart_.IsInterruptAsserted() ? 0x4 : 0x0;
}

UART* Bus::GetUART() {
  return &uart_;
}


