// simctty
// Copyright 2014 Tom Harwood

#include "gtest/gtest.h"

#include <stdio.h>

#include "simctty/exception.h"
#include "simctty/ram.h"

TEST(RAMTest, 20MLoad32) {
  RAM ram;
  Exception exception;

  ram.Store32(0, 0, &exception);

  uint64 sum = 0;
  for (size_t i = 0; i < 20000000; i++) {
    sum += ram.Load32(0, &exception);
  }

  ASSERT_EQ(0U, sum);
}
