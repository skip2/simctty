// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_CPU_H_
#define SIMCTTY_CPU_H_

#include "gtest/gtest_prod.h"

#include <string>

#include "simctty/bus.h"
#include "simctty/mmu.h"
#include "simctty/types.h"

using std::string;

class CPU {
 public:
  CPU(Bus* bus);
  ~CPU();

  bool Run(size_t cycles = 1);

  void Reset();

  uint32 Reg(reg_t reg) const;
  void SetReg(reg_t reg, uint32 value);

  uint32 SpReg(uint16 reg) const;
  void SetSpReg(uint16 reg, uint32 value);
  void SetSupReg(uint32 value);

  uint32 PC() const;
  void SetPC(uint32 pc);

  bool IsFlagSet() const;

  uint64 InstructionRunCount() const;

  // Supervision register bits.
  const static uint32 kSM;     // Supervisor Mode.
  const static uint32 kTEE;    // Tick Timer Exception Enabled.
  const static uint32 kIEE;    // Interrupt Exception Enabled.
  const static uint32 kDCE;    // Data Cache Enable.
  const static uint32 kICE;    // Instruction Cache Enable.
  const static uint32 kDME;    // Data MMU Enable.
  const static uint32 kIME;    // Instruction MMU Enable.
  const static uint32 kLEE;    // Little Endian Enable.
  const static uint32 kCE;     // CID Enable.
  const static uint32 kF;      // Flag (for conditional branching).
  const static uint32 kCY;     // Carry flag.
  const static uint32 kOV;     // Overflow flag.
  const static uint32 kOVE;    // Overflow flag exception.
  const static uint32 kDSX;    // Delay slot exception.
  const static uint32 kEPH;    // Exception Prefix High.
  const static uint32 kFO;     // Fixed One (always set).
  const static uint32 kSUMRA;  // SPRs User Mode Read Access.

  const static size_t kRegCount = 32;

 private:
  // System bus.
  Bus* bus_;
  uint8* raw_;

  // Memory management units.
  MMU immu_;  // Instruction MMU.
  MMU dmmu_;  // Data MMU.

  // Main registers.
  uint32 reg_[kRegCount];

  // Program counter.
  uint32 pc_;

  // Supervision register.
  uint32 sr_;

  // Other group 0 special registers.
  uint32 epcr0_;
  uint32 eear0_;
  uint32 esr0_;

  // Group 9 special registers (programmable interrupt controller).
  uint32 picmr_;
  uint32 picsr_;
  bool pending_interrupt_;

  // Group 10 special registers (tick timer).
  uint32 ttmr_;  // Tick Timer Mode Register.
  uint32 ttcr_;  // Tick Timer Count Register. Overflows by design.

  const static uint32 kTTMRTimePeriodMask = 0xFFFFFFF;
  const static uint32 kTTMRIP = 1 << 28; // TT Interrupt Pending bit.
  const static uint32 kTTMRIE = 1 << 29; // TT Interrupt Enable bit.

  bool in_delay_slot_;
  uint32 delayed_next_pc_;

  uint32 authed_page_;
  uint32 authed_phy_;

  bool RunInstruction(const uint32 instruction);

  void ThrowException(Exception exception, uint32 effective_address = 0);
  void IncrementPC();
  void Jump(uint32 next_pc); // returns true if going to delay slot.
  void SetCompareFlag(bool flag);

  void CheckInterrupts();

  FRIEND_TEST(CPUTest, BusException);
  DISALLOW_COPY_AND_ASSIGN(CPU);
};

#endif  // SIMCTTY_CPU_H_

