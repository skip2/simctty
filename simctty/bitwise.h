// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_BITWISE_H_
#define SIMCTTY_BITWISE_H_

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define B32ENDIANSWAP(x) (x)
#define B32ENDIANSWAPB0 0
#define B32ENDIANSWAPB1 1
#define B32ENDIANSWAPB2 2
#define B32ENDIANSWAPB3 3

#elif  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define B32ENDIANSWAP(x) (\
  ((x>>24)&0x000000ff) | \
  ((x<<8)&0x00ff0000) | \
  ((x>>8)&0x0000ff00) | \
  ((x<<24)&0xff000000))
#define B32ENDIANSWAPB0 3
#define B32ENDIANSWAPB1 2
#define B32ENDIANSWAPB2 1
#define B32ENDIANSWAPB3 0

#else

#error Unknown system endianess.

#endif

#endif  // SIMCTTY_BITWISE_H_

