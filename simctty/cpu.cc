// simctty
// Copyright 2014 Tom Harwood

#include "simctty/cpu.h"

#include <stdio.h>
#include <stdarg.h>

// Supervision register bits.
// S = supported, U = unsupported.
const uint32 CPU::kSM    = 1 << 0;   // S Supervisor Mode.
const uint32 CPU::kTEE   = 1 << 1;   // S Tick Timer Exception Enabled.
const uint32 CPU::kIEE   = 1 << 2;   // S Interrupt Exception Enabled.
const uint32 CPU::kDCE   = 1 << 3;   // U Data Cache Enable.
const uint32 CPU::kICE   = 1 << 4;   // U Instruction Cache Enable.
const uint32 CPU::kDME   = 1 << 5;   // S Data MMU Enable.
const uint32 CPU::kIME   = 1 << 6;   // S Instruction MMU Enable.
const uint32 CPU::kLEE   = 1 << 7;   // U Little Endian Enable.
const uint32 CPU::kCE    = 1 << 8;   // U CID Enable.
const uint32 CPU::kF     = 1 << 9;   // S Flag (for conditional branching).
const uint32 CPU::kCY    = 1 << 10;  // U Carry flag.
const uint32 CPU::kOV    = 1 << 11;  // U Overflow flag.
const uint32 CPU::kOVE   = 1 << 12;  // U Overflow flag exception.
const uint32 CPU::kDSX   = 1 << 13;  // S Delay slot exception.
const uint32 CPU::kEPH   = 1 << 14;  // U Exception Prefix High.
const uint32 CPU::kFO    = 1 << 15;  // S Fixed One (always set).
const uint32 CPU::kSUMRA = 1 << 16;  // U SPRs User Mode Read Access.

CPU::CPU(Bus* bus)
  :
    bus_(bus),
    raw_(bus_->GetRAM()->Raw()),
    immu_(bus_, MMU::kInstruction),
    dmmu_(bus_, MMU::kData) {
  Reset();
}

CPU::~CPU() {
}

void CPU::Reset() {
  // Initalise registers to 0.
  for (size_t i = 0; i < kRegCount; i++) {
    reg_[i] = 0;
  }

  // Initialise program counter.
  pc_ = 0;

  // Enable supervisor mode.
  SetSupReg(kFO | kSM);

  // Initialise other special registers.
  epcr0_ = 0;
  eear0_ = 0;
  esr0_ = 0;

  // Initialise programmable interrupt controller.
  picmr_ = 0;
  picsr_ = 0;
  pending_interrupt_ = false;

  // Initialise tick timer.
  ttmr_ = 0;
  ttcr_ = 0;

  // Reset MMUs.
  dmmu_.Reset();
  immu_.Reset();

  // Not in a delay slot.
  in_delay_slot_ = false;

  // Counters.
  authed_page_ = 0x1;
}

bool CPU::Run(size_t cycles) {
  CheckInterrupts();

  for (size_t i = 0; i < cycles; i++) {
    // Increment tick timer counter register.
    ++ttcr_;

    // Tick timer matches?
    if (ttmr_>>30 == 3 &&
        (ttcr_ & 0x0fffffff) == (ttmr_ & 0x0fffffff) &&
        ((ttmr_>>29) &1) == 1) {
      ttmr_ |= 0x10000000;

      // Tick timer interrupt?
      if (sr_ & kTEE) {
        ThrowException(kExceptionTickTimerInterrupt);
        return true;
      }
    }

    // Counters.
    Exception exception = kExceptionNone;
    uint32 instruction;

    if ((pc_ & 0xffffe000) == authed_page_) {
      // Fast path, ~98.6% of instruction fetches.
      instruction = *reinterpret_cast<uint32*>(raw_ + authed_phy_ + (pc_ & 0x1fff));
    } else {
      // Slow path.
      bool is_ram;
      const uint32 phy_address = immu_.MapAddress(pc_, &exception, sr_& kSM, false, &is_ram);
      if (exception != kExceptionNone) {
        ThrowException(exception, pc_);
        return true;
      }

      if (is_ram) {
        authed_page_ = pc_ & 0xffffe000;
        authed_phy_ = phy_address & 0xffffe000;
        instruction = *reinterpret_cast<uint32*>(raw_ + authed_phy_ + (pc_ & 0x1fff));
      } else {
        // Slowest path for bus attached devices.
        instruction = immu_.Load32(pc_, &exception, sr_ & kSM);
      }
    }

    if (!RunInstruction(instruction)) {
      return false;
    }
  }

  return true;
}

uint32 CPU::Reg(reg_t reg) const {
  return reg_[reg];
}

void CPU::SetReg(reg_t reg, uint32 value) {
  reg_[reg] = value;
}

uint32 CPU::PC() const {
  return pc_;
}

void CPU::SetPC(uint32 pc) {
  pc_ = pc;
}

uint32 CPU::SpReg(reg_t reg) const {
  // Sp registers addresses consist of a 5-bit group and 11-bit index:
  // GGGG GIII IIII IIII.
  const size_t group = (reg & 0xf800) >> 11;
  const size_t index = reg & 0x07ff;

  // Supervisor mode enabled? (SUMRA mode not supported).
  if (!sr_ & kSM) {
    return 0;
  }

  switch (group) {
  case 0:  // System Control and Status registers.
    switch (index) {
    case 0:  // VR: Version register.
      return
        0x0 << 0 |   // REV : Revision (0).
        0x1 << 6 |   // UVRP: Updated Version Registers present (true).
        0x0 << 23 |  // CFG : Configuration template (0).
        0x10 << 24;  // VER : OpenRISC version (minimum: 0x10).
    case 1:  // UPR: Unit Present register.
      // 1 indicates present.
      return
        1 << 0 |  // UP  : UPR Present.
        0 << 1 |  // DCP : Data Cache Present.
        0 << 2 |  // ICP : Instruction Cache Present.
        1 << 3 |  // DMP : Data MMU Present.
        1 << 4 |  // IMP : Instruction MMU Present.
        0 << 5 |  // MP  : MAC Present.
        0 << 6 |  // DUP : Debug Unit Present.
        0 << 7 |  // PCUP: Performance Counters Unit Present.
        0 << 8 |  // PMP : Power Management Present.
        1 << 9 |  // PICP: Programmable Interrupt Controller Present.
        1 << 10;  // TTP : Tick Timer Present.
    case 2:  // CPUCFGR: CPU Configuration register.
      return
        0 << 3  |  // NSGR  : Number of Shadow GPR Files (0).
        0 << 4  |  // CGR   : Custom GPR File (GPR file has 32 registers).
        1 << 5  |  // OB32S : ORBIS32 Supported (supported).
        0 << 6  |  // OB64S : ORBIS64 Supported (not supported).
        0 << 7  |  // OF32S : ORFPX32 Supported (not supported).
        0 << 8  |  // OF64S : ORFPX64 Supported (not supported).
        0 << 9  |  // OV64S : ORVDX64 Supported (not supported).
        0 << 10 |  // ND    : No Delay-Slot (delay-slot implemented).
        1 << 11 |  // AVRP  : Architecture Version Register Present (present).
        0 << 12 |  // EVBAR : Exception Vector Base Addr Register (not-present).
        0 << 13 |  // EVBAR : Exception Vector Base Addr Register (not-present).
        0 << 14;   // AECSRP: AECR & AESR present (not-present).
    case 3:  // DMMUCFGR: Data MMU Configuration register.
      return 6 << 2;
    case 4:  // IMMUCFGR: Instruction MMU Configuration register.
      return 6 << 2;
    case 5:  // DCFCGR: Debug configuration register (not-present).
    case 6:  // ICCGR: Instruction cache configuration register (not-present).
      return 0;
    case 9:   // VR2  : Version register 2.
      return
        0 << 23 |  // VER  : Implementation specific version number.
        0 << 31;   // CPUID: Implementation specific identification number.
    case 10:  // AVR  : Architecture version register.
        return
          0 << 0  |  // Reserved
          0 << 8  |  // REV: Architecture Revision Number.
          0 << 16 |  // MIN: Minor Architecture Version Number.
          1 << 24;   // MAJ: Major Architecture Version Number.
    case 17:  // SR   : Supervision register.
      return sr_;
    case 32:  // EPCR0: Exception PC registers (1 only).
      return epcr0_;
    case 48:  // EEAR0: Exception EA registers (1 only).
      return eear0_;
    case 64:  //  ESR0: Exception SR registers (1 only).
      return esr0_;
    }
    break;
  case 1:  // Data MMU.
    return dmmu_.Reg(index);
  case 2:  // Instruction MMU.
    return immu_.Reg(index);
  case 9:  // Programmable Interrupt Controller.
    switch (index) {
    case 0:  // PIC Mask register.
      return picmr_;
    case 2:  // PIC Status register.
      return picsr_;
    }
    break;
  case 10: // Tick Timer.
    switch (index) {
    case 0:  // Tick Timer Mode register.
      return ttmr_;
    case 1:  // Tick Timer Count register.
      return ttcr_;
    }
    break;
  }

  fprintf(stderr, "GetSpReg: unknown reg (group %ld index %ld) pc=%#x", group, index, pc_);
  return 0;
}

void CPU::SetSpReg(reg_t reg, uint32 value) {
  // Sp registers addresses consist of a 5-bit group and 11-bit index:
  // GGGG GIII IIII IIII.
  const size_t group = (reg & 0xf800) >> 11;
  const size_t index = reg & 0x07ff;

  switch (group) {
  case 0:  // System Control and Status registers.
    switch (index) {
    case 17:  // SR   : Supervision register.
      sr_ = value;
      if (sr_ & (kLEE|kCE|kEPH|kSUMRA)) {
        fprintf(stderr, "Unsupported mode enabled sr_=%#x", sr_);
      }
      immu_.SetIsEnabled(sr_ & kIME);
      dmmu_.SetIsEnabled(sr_ & kDME);

      immu_.ClearFastAuthCache();
      dmmu_.ClearFastAuthCache();
      authed_page_ = 0x1;
      return;
    case 32:  // EPCR0: Exception PC registers (1 only).
      epcr0_ = value;
      return;
    case 48:  // EEAR0: Exception EA registers (1 only).
      eear0_ = value;
      return;
    case 64:  //  ESR0: Exception SR registers (1 only).
      esr0_ = value;
      return;
    }
    break;
  case 1:
    if (dmmu_.SetReg(index, value)) {
      return;
    }
    break;
  case 2:
    if (immu_.SetReg(index, value)) {
      return;
    }
    break;
  case 4:
    switch (index) {
    case 2:
      // IC Block Invalidate Register.
      // Instruction cache is not implemented, ignore writes.
      return;
    };
    break;
  case 9:  // Programmable Interrupt Controller.
    switch (index) {
    case 0:  // PIC Mask register.
      picmr_ = value;
      return;
    case 2:  // PIC Status register.
      picsr_ = value;
      return;
    }
    break;
  case 10: // Tick Timer.
    switch (index) {
    case 0:  // Tick Timer Mode register.
      ttmr_ = value;
      if (ttmr_ >> 30 != 0 && ttmr_ >> 30 != 3) {
        fprintf(stderr, "unknown ttmr mode %d\n", ttmr_ >> 30);
      }
      return;
    case 1:  // Tick Timer Count register.
      ttcr_ = value;
      return;
    }
    break;
  }

  fprintf(stderr, "SetSpReg: unknown reg (group %ld index %ld), value %d", group,
      index, value);
}

void CPU::ThrowException(Exception exception, uint32 effective_address) {
  // Save supervisor register.
  esr0_ = sr_;

  if (in_delay_slot_) {
    esr0_ |= kDSX;
    pc_ -= 4;
    in_delay_slot_ = false;
  }

  SetSupReg(kFO | kSM);

  // Save program counter.
  epcr0_ = pc_;

  // Set PC to exception handling routine.
  pc_ = exceptionHandlers[exception].pc;

  // Set exception effective address register?
  if (exceptionHandlers[exception].setsEEAR) {
    eear0_ = effective_address;
  }
}

void CPU::IncrementPC() {
  if (!in_delay_slot_) {
    pc_ += 4;
  } else {
    pc_ = delayed_next_pc_;
  }
  in_delay_slot_ = false;
}

void CPU::Jump(uint32 next_pc) {
  delayed_next_pc_ = next_pc;
  pc_ += 4;
  in_delay_slot_ = true;
}

void CPU::SetCompareFlag(bool flag) {
  if (flag) {
    sr_ |= kF;
  } else {
    sr_ &= ~kF;
  }
}

bool CPU::IsFlagSet() const {
  return sr_ & kF;
}

#define DECODE_D() d = (instruction >> 21) & kRegMask
#define DECODE_A() a = (instruction >> 16) & kRegMask
#define DECODE_B() b = (instruction >> 11) & kRegMask
#define DECODE_K() k = instruction & 0xffff
#define DECODE_I() i = instruction & 0xffff
#define DECODE_SPLIT_I() i = (instruction&0x7ff) | ((instruction&0x03e00000) >> 10)
#define DECODE_SPLIT_K() k = (instruction&0x7ff) | ((instruction&0x03e00000) >> 10)
#define DECODE_OPCODE11() opcode11 = (instruction >> 21) & 0x7ff;
#define DECODE_N() n = static_cast<int32>(instruction << 6) >> 4;
#define DECODE_OPCODE11() opcode11 = (instruction >> 21) & 0x7ff;
#define DECODE_L() l = instruction & 0x1f;
#define DECODE_F() f = (instruction >> 6) & 0x3;
#define DECODE_FF() ff = (instruction&0xf) | ((instruction&0x3c0) >> 2);

bool CPU::RunInstruction(const uint32 instruction) {
  Exception exception = kExceptionNone;

  // Run instruction.
  const uint8 opcode = (instruction >> 26) & 0x3f;

  reg_t d;
  reg_t a;
  reg_t b;
  uint8 l;
  uint8 f;
  uint8 ff;
  uint16 k;
  uint32 ea;
  uint32 value;
  int16 i;
  int32 n;
  uint16 opcode11;

  switch (opcode) {
  case 0x00:  // 0000 00NN NNNN NNNN NNNN NNNN NNNN NNNN l.j
    DECODE_N();
    Jump(pc_ + n);
    break;
  case 0x01:  // 0000 01NN NNNN NNNN NNNN NNNN NNNN NNNN l.jal
    DECODE_N();
    reg_[9] = pc_ + 8;
    Jump(pc_ + n);
    break;
  case 0x03:  // 0000 11NN NNNN NNNN NNNN NNNN NNNN NNNN l.bnf
    if (!(sr_ & kF)) {
      DECODE_N();
      Jump(pc_ + n);
    } else {
      IncrementPC();
    }
    break;
  case 0x04:  // 0001 00NN NNNN NNNN NNNN NNNN NNNN NNNN l.bf
    if (sr_ & kF) {
      DECODE_N();
      Jump(pc_ + n);
    } else {
      IncrementPC();
    }
    break;
  case 0x05:  // 0001 0101 ---- ---- KKKK KKKK KKKK KKKK l.nop
    DECODE_K();
    if ((sr_ & kSM) && k == 1) {
      return false;
    } else {
      IncrementPC();
    }
    break;
  case 0x06:  // 0001 10DD DDD- ---0 KKKK KKKK KKKK KKKK l.movhi
    DECODE_D();
    DECODE_K();
    reg_[d] = k << 16;
    IncrementPC();
    break;
  case 0x08:  // Multiple instructions.
    DECODE_K();
    switch ((instruction >> 16) & 0xffff) {
    case 0x2000:  // 0010 0000 0000 0000 KKKK KKKK KKKK KKKK l.sys
      pc_ += 4;
      ThrowException(kExceptionSystemCall);
      break;
    case 0x2100:  // 0010 0001 0000 0000 KKKK KKKK KKKK KKKK l.trap Trap
      IncrementPC();
      return false;
    default:
      ThrowException(kExceptionIllegalInstruction, pc_);
      break;
    }
    break;
  case 0x09:  // 0010 01-- ---- ---- ---- ---- ---- ---- l.rfe
    pc_ = epcr0_;
    SetSupReg(esr0_);
    in_delay_slot_ = false;
    break;
  case 0x11:  // 0100 01-- ---- ---- BBBB B--- ---- ---- l.jr
    DECODE_B();
    Jump(reg_[b]);
    break;
  case 0x12:  // 0100 10-- ---- ---- BBBB B--- ---- ---- l.jalr
    DECODE_B();
    reg_[9] = pc_ + 8;
    Jump(reg_[b]);
    break;
  case 0x21:  // 1000 01DD DDDA AAAA IIII IIII IIII IIII l.lwz
    DECODE_D();
    DECODE_A();
    DECODE_I();
    ea = reg_[a] + i;
    value = dmmu_.Load32(ea, &exception, sr_ & kSM);

    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      reg_[d] = value;
      IncrementPC();
    }
    break;
  case 0x23: // 1000 11DD DDDA AAAA IIII IIII IIII IIII l.lbz
    DECODE_D();
    DECODE_A();
    DECODE_I();
    ea = reg_[a] + i;
    value = dmmu_.Load8(ea, &exception, sr_ & kSM);
    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      reg_[d] = value;
      IncrementPC();
    }
    break;
  case 0x24:  // 1001 00DD DDDA AAAA IIII IIII IIII IIII l.lbs
    DECODE_D();
    DECODE_A();
    DECODE_I();
    ea = reg_[a] + i;
    value = static_cast<int8>(dmmu_.Load8(ea, &exception, sr_ & kSM));
    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      reg_[d] = value;
      IncrementPC();
    }
    break;
  case 0x25:  // 1001 01DD DDDA AAAA IIII IIII IIII IIII l.lhz
    DECODE_D();
    DECODE_A();
    DECODE_I();
    ea = reg_[a] + i;
    value = dmmu_.Load16(ea, &exception, sr_ & kSM);
    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      reg_[d] = value;
      IncrementPC();
    }
    break;
  case 0x26:  // 1001 10DD DDDA AAAA IIII IIII IIII IIII l.lhs
    DECODE_D();
    DECODE_A();
    DECODE_I();
    ea = reg_[a] + i;
    value = static_cast<int16>(dmmu_.Load16(ea, &exception, sr_ & kSM));
    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      reg_[d] = value;
      IncrementPC();
    }
    break;
  case 0x27:  // 1001 11DD DDDA AAAA IIII IIII IIII IIII l.addi
    DECODE_D();
    DECODE_A();
    DECODE_I();
    reg_[d] = reg_[a] + i;
    IncrementPC();
    break;
  case 0x29:  // 1010 01DD DDDA AAAA KKKK KKKK KKKK KKKK l.andi
    DECODE_D();
    DECODE_A();
    DECODE_K();
    reg_[d] = reg_[a] & k;
    IncrementPC();
    break;
  case 0x2a:  // 1010 10DD DDDA AAAA KKKK KKKK KKKK KKKK l.ori
    DECODE_D();
    DECODE_A();
    DECODE_K();
    reg_[d] = reg_[a] | k;
    IncrementPC();
    break;
  case 0x2b:  // 1010 11DD DDDA AAAA IIII IIII IIII IIII l.xori
    DECODE_D();
    DECODE_A();
    DECODE_I();
    reg_[d] = reg_[a] ^ i;
    IncrementPC();
    break;
  case 0x2d:  // 1011 01DD DDDA AAAA KKKK KKKK KKKK KKKK l.mfspr
    DECODE_D();
    DECODE_A();
    DECODE_K();
    reg_[d] = SpReg(reg_[a] | k);
    IncrementPC();
    break;
  case 0x2e:  // Multiple instructions.
    DECODE_F();
    DECODE_D();
    DECODE_A();
    DECODE_L();
    switch (f) {
    case 0:  // 1011 10DD DDDA AAAA ---- ---- 00LL LLLL l.slli
      reg_[d] = reg_[a] << l;
      IncrementPC();
      break;
    case 1:  // 1011 10DD DDDA AAAA ---- ---- 01LL LLLL l.srli
      reg_[d] = reg_[a] >> l;
      IncrementPC();
      break;
    case 2:  // 1011 10DD DDDA AAAA ---- ---- 10LL LLLL l.srai
      reg_[d] = static_cast<int32>(reg_[a]) >> l;
      IncrementPC();
      break;
    default:
      ThrowException(kExceptionIllegalInstruction, pc_);
      break;
    }
    break;
  case 0x2f:  // Multiple instructions.
    DECODE_A();
    DECODE_I();
    DECODE_OPCODE11();
    switch (opcode11) {
    case 0x5e0:  // 1011 11 00 000A AAAA IIII IIII IIII IIII l.sfeqi
      SetCompareFlag(static_cast<int32>(reg_[a]) == i);
      IncrementPC();
      break;
    case 0x5e1:  // 1011 1100 001A AAAA IIII IIII IIII IIII l.sfnei
      SetCompareFlag(static_cast<int32>(reg_[a]) != i);
      IncrementPC();
      break;
    case 0x5e2:  // 1011 1100 010A AAAA IIII IIII IIII IIII l.sfgtui
      SetCompareFlag(reg_[a] > static_cast<uint32>(i));
      IncrementPC();
      break;
    case 0x5e3:  // 1011 1100 011A AAAA IIII IIII IIII IIII l.sfgeui
      SetCompareFlag(reg_[a] >= static_cast<uint32>(i));
      IncrementPC();
      break;
    case 0x5e4:  // 1011 1100 100A AAAA IIII IIII IIII IIII l.sfltui
      SetCompareFlag(reg_[a] < static_cast<uint32>(i));
      IncrementPC();
      break;
    case 0x5e5:  // 1011 1100 101A AAAA IIII IIII IIII IIII l.sfleui
      SetCompareFlag(reg_[a] <= static_cast<uint32>(i));
      IncrementPC();
      break;
    case 0x5ea:  // 1011 1101 010A AAAA IIII IIII IIII IIII l.sfgtsi
      SetCompareFlag(static_cast<int32>(reg_[a]) > i);
      IncrementPC();
      break;
    case 0x5eb:  // 1011 1101 011A AAAA IIII IIII IIII IIII l.sfgesi
      SetCompareFlag(static_cast<int32>(reg_[a]) >= i);
      IncrementPC();
      break;
    case 0x5ec:  // 1011 1101 100A AAAA IIII IIII IIII IIII l.sfltsi
      SetCompareFlag(static_cast<int32>(reg_[a]) < i);
      IncrementPC();
      break;
    case 0x5ed:  // 1011 1101 101A AAAA IIII IIII IIII IIII l.sflesi
      SetCompareFlag(static_cast<int32>(reg_[a]) <= i);
      IncrementPC();
      break;
    default:
      ThrowException(kExceptionIllegalInstruction, pc_);
      break;
    }
    break;
  case 0x30:  // 1100 00KK KKKA AAAA BBBB BKKK KKKK KKKK l.mtspr
    DECODE_A();
    DECODE_B();
    DECODE_SPLIT_K();
    SetSpReg(reg_[a] | k, reg_[b]);
    if (ttmr_ & 0x10000000 && (sr_ & kTEE)) {
      // Pending tick timer interrupt now available?
      ThrowException(kExceptionTickTimerInterrupt);
    } else {
      IncrementPC();
    }
    break;
  case 0x35:  // 1101 01II IIIA AAAA BBBB BIII IIII IIII l.sw
    DECODE_SPLIT_I();
    DECODE_A();
    DECODE_B();
    ea = reg_[a] + i;
    dmmu_.Store32(ea, reg_[b], &exception, sr_ & kSM);
    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      IncrementPC();
    }
    break;
  case 0x36:  // 1101 10II IIIA AAAA BBBB BIII IIII IIII l.sb
    DECODE_SPLIT_I();
    DECODE_A();
    DECODE_B();
    ea = reg_[a] + i;
    dmmu_.Store8(ea, reg_[b] & 0xff, &exception, sr_ & kSM);
    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      IncrementPC();
    }
    break;
  case 0x37:  // 1101 11II IIIA AAAA BBBB BIII IIII IIII l.sh
    DECODE_SPLIT_I();
    DECODE_A();
    DECODE_B();
    ea = reg_[a] + i;
    dmmu_.Store16(ea, reg_[b] & 0xffff, &exception, sr_ & kSM);
    if (exception != kExceptionNone) {
      ThrowException(exception, ea);
    } else {
      IncrementPC();
    }
    break;
  case 0x38:  // Multiple instructions.
    DECODE_FF();
    DECODE_D();
    DECODE_A();
    DECODE_B();
    switch (ff) {
    case 0x00:  // 1110 00DD DDDA AAAA BBBB B-00 ---- 0000 l.add
      reg_[d] = reg_[a] + reg_[b];
      IncrementPC();
      break;
    case 0x02:  // 1110 00DD DDDA AAAA BBBB B-00 ---- 0010 l.sub
      reg_[d] = reg_[a] - reg_[b];
      IncrementPC();
      break;
    case 0x03:  // 1110 00DD DDDA AAAA BBBB B-00 ---- 0011 l.and
      reg_[d] = reg_[a] & reg_[b];
      IncrementPC();
      break;
    case 0x04:  // 1110 00DD DDDA AAAA BBBB B-00 ---- 0100 l.or
      reg_[d] = reg_[a] | reg_[b];
      IncrementPC();
      break;
    case 0x05:  // 1110 00DD DDDA AAAA BBBB B-00 ---- 0101 l.xor
      reg_[d] = reg_[a] ^ reg_[b];
      IncrementPC();
      break;
    case 0x08:  // 1110 00DD DDDA AAAA BBBB B-00 00-- 1000 l.sll
      reg_[d] = reg_[a] << (reg_[b]&0x1f);
      IncrementPC();
      break;
    case 0x0f:  // 1110 00DD DDDA AAAA BBBB B-00 ---- 1111 l.ff1
      value = 0;
      for (uint8 i = 0; i < 32; i++) {
        if (reg_[a] & (1 << i)) {
          value = i + 1;
          break;
        }
      }
      reg_[d] = value;
      IncrementPC();
      break;
    case 0x18:  // 1110 00DD DDDA AAAA BBBB B-00 01-- 1000 l.srl
      reg_[d] = reg_[a] >> (reg_[b]&0x1f);
      IncrementPC();
      break;
    case 0x28:  // 1110 00DD DDDA AAAA BBBB B-00 10-- 1000 l.sra
      reg_[d] = static_cast<int32>(reg_[a]) >> (reg_[b]&0x1f);
      IncrementPC();
      break;
    case 0x4f:  // 1110 00DD DDDA AAAA BBBB B-01 ---- 1111 l.fl1
      value = 0;
      for (int8 i = 31; i >= 0; i--) {
        if (reg_[a] & (1 << i)) {
          value = i + 1;
          break;
        }
      }
      reg_[d] = value;
      IncrementPC();
      break;
    case 0xc6:  // 1110 00DD DDDA AAAA BBBB B-11 ---- 0110 l.mul
      reg_[d] = static_cast<int32>(reg_[a]) * static_cast<int32>(reg_[b]);
      IncrementPC();
      break;
    case 0xc9:  // 1110 00DD DDDA AAAA BBBB B-11 ---- 1001 l.div
      if (reg_[b] == 0) {
        reg_[d] = 0;
      } else {
        reg_[d] = static_cast<int32>(reg_[a]) / static_cast<int32>(reg_[b]);
      }
      IncrementPC();
      break;
    case 0xca:  // 1110 00DD DDDA AAAA BBBB B-11 ---- 1010 l.divu
      if (reg_[b] == 0) {
        reg_[d] = 0;
      } else {
        reg_[d] = reg_[a] / reg_[b];
      }
      IncrementPC();
      break;
    default:
      ThrowException(kExceptionIllegalInstruction, pc_);
      break;
    }
    break;
  case 0x39:  // Multiple instructions.
    DECODE_A();
    DECODE_B();
    DECODE_OPCODE11();
    switch (opcode11) {
    case 0x720:  // 1110 0100 000A AAAA BBBB B--- ---- ---- l.sfeq
      SetCompareFlag(reg_[a] == reg_[b]);
      IncrementPC();
      break;
    case 0x721:  // 1110 0100 001A AAAA BBBB B--- ---- ---- l.sfne
      SetCompareFlag(reg_[a] != reg_[b]);
      IncrementPC();
      break;
    case 0x722:  // 1110 0100 010A AAAA BBBB B--- ---- ---- l.sfgtu
      SetCompareFlag(reg_[a] > reg_[b]);
      IncrementPC();
      break;
    case 0x723:  // 1110 0100 011A AAAA BBBB B--- ---- ---- l.sfgeu
      SetCompareFlag(reg_[a] >= reg_[b]);
      IncrementPC();
      break;
    case 0x724:  // 1110 0100 100A AAAA BBBB B--- ---- ---- l.sfltu
      SetCompareFlag(reg_[a] < reg_[b]);
      IncrementPC();
      break;
    case 0x725:  // 1110 0100 101A AAAA BBBB B--- ---- ---- l.sfleu
      SetCompareFlag(reg_[a] <= reg_[b]);
      IncrementPC();
      break;
    case 0x72a:  // 1110 0101 010A AAAA BBBB B--- ---- ---- l.sfgts
      SetCompareFlag(static_cast<int32>(reg_[a]) > static_cast<int32>(reg_[b]));
      IncrementPC();
      break;
    case 0x72b:  // 1110 0101 011A AAAA BBBB B--- ---- ---- l.sfges
      SetCompareFlag(static_cast<int32>(reg_[a]) >= static_cast<int32>(reg_[b]));
      IncrementPC();
      break;
    case 0x72c:  // 1110 0101 100A AAAA BBBB B--- ---- ---- l.sflts
      SetCompareFlag(static_cast<int32>(reg_[a]) < static_cast<int32>(reg_[b]));
      IncrementPC();
      break;
    case 0x72d:  // 1110 0101 101A AAAA BBBB B--- ---- ---- l.sfles
      SetCompareFlag(static_cast<int32>(reg_[a]) <= static_cast<int32>(reg_[b]));
      IncrementPC();
      break;
    default:
      ThrowException(kExceptionIllegalInstruction, pc_);
      break;
    }
    break;
  default:
    ThrowException(kExceptionIllegalInstruction, pc_);
    break;
  }
  return true;
}

void CPU::SetSupReg(uint32 value) {
  const uint16 kSpRegSup = 0<<11 | 17;    // Supervisor register.
  SetSpReg(kSpRegSup, value);
}

void CPU::CheckInterrupts() {
  const uint32 new_picsr = bus_->Interrupts();
  if (new_picsr != picsr_) {
    picsr_ = new_picsr;
    pending_interrupt_ = picsr_ != 0;
  }

  if (sr_ & kIEE && pending_interrupt_ && picsr_ & picmr_) {
    ThrowException(kExceptionExternalInterrupt);
    pending_interrupt_ = false;
  }
}

