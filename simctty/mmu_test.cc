// simctty
// Copyright 2014 Tom Harwood

#include "gtest/gtest.h"

#include <stdio.h>

#include "simctty/bus.h"
#include "simctty/exception.h"
#include "simctty/mmu.h"
#include "simctty/ram.h"

TEST(MMUTest, 20MLoad32) {
  Bus bus;
  MMU mmu(&bus, MMU::kInstruction);
  Exception exception;

  bus.GetRAM()->Store32(0, 0, &exception);

  uint64 sum = 0;
  for (size_t i = 0; i < 20000000; i++) {
    sum += mmu.Load32(0, &exception, true);
  }

  ASSERT_EQ(0U, sum);
}

