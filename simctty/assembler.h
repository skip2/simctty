// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_ASSEMBLER_H_
#define SIMCTTY_ASSEMBLER_H_

#include <string>
#include <vector>

#include "simctty/bus.h"
#include "simctty/types.h"

using std::string;
using std::vector;

class Assembler {
 public:
  Assembler();
  ~Assembler();

  const uint8* Instructions() const;
  size_t Size() const;
  size_t InstructionCount() const;

  void Clear();

  void l_j(uint32 n);
  void l_jal(uint32 n);
  void l_bnf(uint32 n);
  void l_bf(uint32 n);

  void l_nop(uint16 k = 0U);
  void l_movhi(reg_t d, uint16 k);

  void l_sys(uint16 k = 0);
  void l_trap(uint16 k = 0);

  void l_rfe();

  void l_jr(reg_t b);
  void l_jalr(reg_t b);

  void l_lwz(reg_t d, reg_t a, int16 i);
  void l_lbs(reg_t d, reg_t a, int16 i);
  void l_lbz(reg_t d, reg_t a, int16 i);
  void l_lhz(reg_t d, reg_t a, int16 i);
  void l_lhs(reg_t d, reg_t a, int16 i);
  void l_addi(reg_t d, reg_t a, int16 i);
  void l_andi(reg_t d, reg_t a, uint16 k);
  void l_ori(reg_t d, reg_t a, uint16 k);
  void l_xori(reg_t d, reg_t a, int16 i);
  void l_mfspr(reg_t d, reg_t a, uint16 k);
  void l_slli(reg_t d, reg_t a, uint8 l);
  void l_srli(reg_t d, reg_t a, uint8 l);
  void l_srai(reg_t d, reg_t a, uint8 l);

  void l_sfeqi(reg_t a, int16 i);
  void l_sfnei(reg_t a, int16 i);
  void l_sfgtui(reg_t a, int16 i);
  void l_sfgeui(reg_t a, int16 i);
  void l_sfltui(reg_t a, int16 i);
  void l_sfleui(reg_t a, int16 i);
  void l_sfgtsi(reg_t a, int16 i);
  void l_sfgesi(reg_t a, int16 i);
  void l_sfltsi(reg_t a, int16 i);
  void l_sflesi(reg_t a, int16 i);

  void l_mtspr(reg_t a, reg_t b, uint16 k);
  void l_sw(reg_t a, reg_t b, int16 i);
  void l_sb(reg_t a, reg_t b, int16 i);
  void l_sh(reg_t a, reg_t b, int16 i);

  void l_add(reg_t d, reg_t a, reg_t b);
  void l_sub(reg_t d, reg_t a, reg_t b);
  void l_and(reg_t d, reg_t a, reg_t b);
  void l_or(reg_t d, reg_t a, reg_t b);
  void l_xor(reg_t d, reg_t a, reg_t b);
  void l_ff1(reg_t d, reg_t a, reg_t b);
  void l_sll(reg_t d, reg_t a, reg_t b);
  void l_srl(reg_t d, reg_t a, reg_t b);
  void l_sra(reg_t d, reg_t a, reg_t b);
  void l_fl1(reg_t d, reg_t a, reg_t b);

  void l_mul(reg_t d, reg_t a, reg_t b);
  void l_div(reg_t d, reg_t a, reg_t b);
  void l_divu(reg_t d, reg_t a, reg_t b);

  void l_sfeq(reg_t a, reg_t b);
  void l_sfne(reg_t a, reg_t b);
  void l_sfgtu(reg_t a, reg_t b);
  void l_sfgeu(reg_t a, reg_t b);
  void l_sfltu(reg_t a, reg_t b);
  void l_sfleu(reg_t a, reg_t b);
  void l_sfgts(reg_t a, reg_t b);
  void l_sfges(reg_t a, reg_t b);
  void l_sflts(reg_t a, reg_t b);
  void l_sfles(reg_t a, reg_t b);

  void SetAddress(uint32 address);
  void Data(uint32 data);

  typedef void(Assembler::*RegOpFunction)(reg_t d, reg_t a, reg_t b);
  typedef void(Assembler::*SFFunction)(reg_t a, reg_t b);
  typedef void(Assembler::*SFIFunction)(reg_t a, int16 i);

 private:
  void AppendInstructionTypeN(uint8 opcode, uint32 n);
  void AppendInstructionTypeEXK(uint8 opcode, uint16 k);
  void AppendInstructionTypeDFK(uint8 opcode, reg_t d, bool f, uint16 k);
  void AppendInstructionTypeK(uint16 opcode, uint16 k);
  void AppendInstructionTypeX(uint8 opcode);
  void AppendInstructionTypeB(uint8 opcode, reg_t b);
  void AppendInstructionTypeXAI(uint8 opcode, reg_t a, int16 i);
  void AppendInstructionTypeDAI(uint8 opcode, reg_t d, reg_t a, int16 i);
  void AppendInstructionTypeDAK(uint8 opcode, reg_t d, reg_t a, uint16 k);
  void AppendInstructionTypeDAFL(uint8 opcode, reg_t d, reg_t a, uint8 f, uint8 l);
  void AppendInstructionTypeAI(uint16 opcode, reg_t a, int16 i);
  void AppendInstructionTypeABK(uint8 opcode, reg_t a, reg_t b, uint16 k);
  void AppendInstructionTypeABI(uint8 opcode, reg_t a, reg_t b, int16 i);
  void AppendInstructionTypeDABFF(uint8 opcode, reg_t d, reg_t a, reg_t b, uint8 f1, uint8 f2);
  void AppendInstructionTypeAB(uint16 opcode, reg_t a, reg_t b);
  void AppendInstruction(uint32 system_endian_instruction);

  // Instructions/data in big-endian.
  vector<uint8> instructions_;

  DISALLOW_COPY_AND_ASSIGN(Assembler);
};


const reg_t kR0 = 0;
const reg_t kR1 = 1;
const reg_t kR2 = 2;
const reg_t kR3 = 3;
const reg_t kR4 = 4;
const reg_t kR5 = 5;
const reg_t kR6 = 6;
const reg_t kR7 = 7;
const reg_t kR8 = 8;
const reg_t kR9 = 9;
const reg_t kR10 = 10;
const reg_t kR11 = 11;
const reg_t kR12 = 12;
const reg_t kR13 = 13;
const reg_t kR14 = 14;
const reg_t kR15 = 15;
const reg_t kR16 = 16;
const reg_t kR17 = 17;
const reg_t kR18 = 18;
const reg_t kR19 = 19;
const reg_t kR20 = 20;
const reg_t kR21 = 21;
const reg_t kR22 = 22;
const reg_t kR23 = 23;
const reg_t kR24 = 24;
const reg_t kR25 = 25;
const reg_t kR26 = 26;
const reg_t kR27 = 27;
const reg_t kR28 = 28;
const reg_t kR29 = 29;
const reg_t kR30 = 30;
const reg_t kR31 = 31;

#endif  // SIMCTTY_ASSEMBLER_H_

