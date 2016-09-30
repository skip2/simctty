// simctty
// Copyright 2014 Tom Harwood

#include "simctty/ram.h"

#include <string.h>

#include "simctty/bitwise.h"

RAM::RAM()
  :
    BusDevice(),
    size_(kMaxRamAddress + 1),
    ram_(new uint8[size_]) {
}

RAM::~RAM() {
  delete ram_;
}

uint32 RAM::Size() const {
  return size_;
}

uint8* RAM::Raw() {
  return ram_;
}

bool RAM::LoadImage(const uint8* data, size_t len, size_t offset) {
  if (offset + len > size_) {
    return false;
  } else if ((offset % 4) != 0) {
    fprintf(stderr, "bad offset\n");
  }

  size_t i = 0;
  for (; i < len; i += 4) {
    ram_[offset + i + 0] = data[i+B32ENDIANSWAPB0];
    ram_[offset + i + 1] = data[i+B32ENDIANSWAPB1];
    ram_[offset + i + 2] = data[i+B32ENDIANSWAPB2];
    ram_[offset + i + 3] = data[i+B32ENDIANSWAPB3];
  }

  if (len % 4 != 0) {
    uint8 last_data[4] = {0};

    size_t k = 0;
    for (size_t j = len&~0x3; j < len; j++) {
      last_data[k++] = data[j];
    }

    fprintf(stderr, "silly swap case\n");
    exit(1);
    ram_[offset + i + 0] = last_data[B32ENDIANSWAPB0];
    ram_[offset + i + 1] = last_data[B32ENDIANSWAPB1];
    ram_[offset + i + 2] = last_data[B32ENDIANSWAPB2];
    ram_[offset + i + 3] = last_data[B32ENDIANSWAPB3];
  }

  return true;
}

uint8 RAM::Load8(uint32 address, Exception* exception) const {
  *exception = kExceptionNone;
  return ram_[address ^ 0x3];
}


void RAM::Store8(uint32 address, uint8 value, Exception* exception) {
  *exception = kExceptionNone;
  ram_[address ^ 0x3] = value;
}

uint16 RAM::Load16(uint32 address, Exception* exception) const {
  *exception = kExceptionNone;

  return *(reinterpret_cast<const uint16*>(ram_ + (address^0x2)));
}

void RAM::Store16(uint32 address, uint16 value, Exception* exception) {
  *exception = kExceptionNone;
  uint16* u16address = reinterpret_cast<uint16*>(ram_ + (address^0x2));
  *u16address = value;
}

uint32 RAM::Load32(uint32 address, Exception* exception) const {
  const uint32* u32address = reinterpret_cast<const uint32*>(ram_ + address);

  *exception = kExceptionNone;
  return *u32address;
}

void RAM::Store32(uint32 address, uint32 value, Exception* exception) {
  uint32* u32address = reinterpret_cast<uint32*>(ram_ + address);

  *exception = kExceptionNone;
  *u32address = value;
}

void RAM::DumpU8(const char* filename) const {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    return;
  }

  for (size_t i = 0; i < kMaxRamAddress; i++) {
    uint8 val = ram_[i^0x3];
    fwrite(&val, 1, 1, file);
  }

  fclose(file);
}



