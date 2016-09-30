// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_EXCEPTION_H_
#define SIMCTTY_EXCEPTION_H_

#include <string>
#include <vector>

#include "simctty/types.h"

enum Exception {
  kExceptionNone = 0,
  kExceptionReset,
  kExceptionBusError,
  kExceptionDataPageFault,
  kExceptionInstructionPageFault,
  kExceptionTickTimerInterrupt,
  kExceptionAlignment,
  kExceptionIllegalInstruction,
  kExceptionExternalInterrupt,
  kExceptionDTLBMiss,
  kExceptionITLBMiss,
  kExceptionRange,
  kExceptionSystemCall,
  kExceptionFloatingPoint,
  kExceptionTrap,
};

struct ExceptionHandler {
  uint32 pc;
  bool setsEEAR;
};

const ExceptionHandler exceptionHandlers[] = {
  {0,     false},  // kExceptionNone
  {0x100, false},  // kExceptionReset
  {0x200, true},   // kExceptionBusError
  {0x300, true},   // kExceptionDataPageFault
  {0x400, true},   // kExceptionInstructionPageFault
  {0x500, false},  // kExceptionTickTimerInterrupt
  {0x600, true},   // kExceptionAlignment
  {0x700, true},   // kExceptionIllegalInstruction
  {0x800, false},  // kExceptionExternalInterrupt
  {0x900, true},   // kExceptionDTLBMiss
  {0xa00, true},   // kExceptionITLBMiss
  {0xb00, false},  // kExceptionRange
  {0xc00, false},  // kExceptionSystemCall
  {0xd00, false},  // kExceptionFloatingPoint
  {0xe00, false},  // kExceptionTrap
};

#endif  // SIMCITY_EXCEPTION_H_

