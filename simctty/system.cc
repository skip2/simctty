// simctty
// Copyright 2014 Tom Harwood

#include "simctty/system.h"

#include <stdio.h>

#include <fstream>      // std::ifstream
#include <string>

using std::ifstream;
using std::string;

System::System()
  :
    bus_(),
    cpu_(&bus_) {
}

System::~System() {
}

CPU* System::GetCPU() {
  return &cpu_;
}

const RAM* System::GetRAM() const {
  return bus_.GetRAM();
}

RAM* System::GetRAM() {
  return bus_.GetRAM();
}

Bus* System::GetBus() {
  return &bus_;
}

UART* System::GetUART() {
  return bus_.GetUART();
}

bool System::LoadImageFile(const char* filename, uint32 start_address) {
  cpu_.Reset();
  cpu_.SetPC(start_address);

  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Can't open %s\n", filename);
    return false;
  }

  const size_t kBufferSize = 1024;
  uint8 buffer[kBufferSize];
  size_t offset = 0;
  size_t len;
  while ((len = fread(buffer, sizeof(uint8), kBufferSize, file)) != 0) {
    bus_.GetRAM()->LoadImage(buffer, len, offset);
    offset += len;
  }

  fclose(file);

  fprintf(stderr, "Loaded %ld bytes from %s\n", offset, filename);

  return true;
}

size_t System::LoadImage(const uint8* data, size_t length, uint32 start_address) {
  cpu_.Reset();
  cpu_.SetPC(start_address);

  bus_.GetRAM()->LoadImage(data, length, 0);
  return length;
}

bool System::Run(size_t cycles) {
  return cpu_.Run(cycles);
}

