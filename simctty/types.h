// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_TYPES_H_
#define SIMCTTY_TYPES_H_

#include <stdlib.h>

#define DISALLOW_COPY_AND_ASSIGN(T) \
  T(const T&); \
  void operator=(const T&)

#define UNUSED(T) \
  ((void)T)

#define UNREACHABLE() \
  abort();

#define NOT_IMPLEMENTED() \
  abort();

#define ARRAYSIZE(T) \
  (sizeof(T) / sizeof(T[0]))

#define STATIC_ASSERT(condition, name) \
  typedef char static_assertion_failed_##name[(condition) ? 1 : -1]

typedef unsigned char uint8;  // NOLINT(runtime/int)
typedef unsigned short uint16;  // NOLINT(runtime/int)
typedef unsigned int uint32;  // NOLINT(runtime/int)
typedef unsigned long long uint64;  // NOLINT(runtime/int)
typedef char int8;  // NOLINT(runtime/int)
typedef short int16;  // NOLINT(runtime/int)
typedef int int32;  // NOLINT(runtime/int)
typedef long long int64;  // NOLINT(runtime/int)

typedef unsigned short reg_t;  // NOLINT(runtime/int)

STATIC_ASSERT(sizeof(uint8) == 1, uint8_is_1_byte);
STATIC_ASSERT(sizeof(uint16) == 2, uint16_is_2_bytes);
STATIC_ASSERT(sizeof(uint32) == 4, uint32_is_4_bytes);
STATIC_ASSERT(sizeof(uint64) == 8, uint64_is_8_bytes);
STATIC_ASSERT(sizeof(int8) == 1, int8_is_1_byte);
STATIC_ASSERT(sizeof(int16) == 2, int16_is_2_bytes);
STATIC_ASSERT(sizeof(int32) == 4, int32_is_4_bytes);
STATIC_ASSERT(sizeof(int64) == 8, int64_is_8_bytes);

STATIC_ASSERT(sizeof(reg_t) == 2, reg_t_is_2_bytes);

#define UINT16_MAXSTRLEN 6

const uint16 kuint16min = 0;
const uint16 kuint16max = 0xffff;

const uint16 kRegMask = 0x1f;

#endif  // SIMCTTY_TYPES_H_

