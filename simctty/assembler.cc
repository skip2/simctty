// simctty
// Copyright 2014 Tom Harwood

#include "simctty/assembler.h"

#include "simctty/bitwise.h"

Assembler::Assembler() {
}

Assembler::~Assembler() {
}

const uint8* Assembler::Instructions() const {
  return instructions_.data();
}

size_t Assembler::Size() const {
  return instructions_.size();
}

size_t Assembler::InstructionCount() const {
  return instructions_.size() / 4;
}

void Assembler::Clear() {
  instructions_.clear();
}

// 0000 00NN NNNN NNNN NNNN NNNN NNNN NNNN l.j
void Assembler::l_j(uint32 n) {
  AppendInstructionTypeN(0x0, n);
}

// 0000 01NN NNNN NNNN NNNN NNNN NNNN NNNN l.jal
void Assembler::l_jal(uint32 n) {
  AppendInstructionTypeN(0x1, n);
}

// 0000 11NN NNNN NNNN NNNN NNNN NNNN NNNN l.bnf
void Assembler::l_bnf(uint32 n) {
  AppendInstructionTypeN(0x3, n);
}

// 0001 00NN NNNN NNNN NNNN NNNN NNNN NNNN l.bf
void Assembler::l_bf(uint32 n) {
  AppendInstructionTypeN(0x4, n);
}

// 0001 0101 ---- ---- KKKK KKKK KKKK KKKK l.nop
void Assembler::l_nop(uint16 k) {
  AppendInstructionTypeEXK(0x15, k);
}

// 0001 10DD DDD- ---0 KKKK KKKK KKKK KKKK l.movhi
void Assembler::l_movhi(reg_t d, uint16 k) {
  AppendInstructionTypeDFK(0x6, d, false, k);
}

// 0010 0000 0000 0000 KKKK KKKK KKKK KKKK l.sys
void Assembler::l_sys(uint16 k) {
  AppendInstructionTypeK(0x2000, k);
}

// 0010 0001 0000 0000 KKKK KKKK KKKK KKKK l.trap Trap
void Assembler::l_trap(uint16 k) {
  AppendInstructionTypeK(0x2100, k);
}

// 0010 01-- ---- ---- ---- ---- ---- ---- l.rfe
void Assembler::l_rfe() {
  AppendInstructionTypeX(0x9);
}

// 0100 01-- ---- ---- BBBB B--- ---- ---- l.jr
void Assembler::l_jr(reg_t b) {
  AppendInstructionTypeB(0x11, b);
}

// 0100 10-- ---- ---- BBBB B--- ---- ---- l.jalr
void Assembler::l_jalr(reg_t b) {
  AppendInstructionTypeB(0x12, b);
}

// 1000 01DD DDDA AAAA IIII IIII IIII IIII l.lwz
void Assembler::l_lwz(reg_t d, reg_t a, int16 i) {
  AppendInstructionTypeDAI(0x21, d, a, i);
}

// 1000 11DD DDDA AAAA IIII IIII IIII IIII l.lbz
void Assembler::l_lbz(reg_t d, reg_t a, int16 i) {
  AppendInstructionTypeDAI(0x23, d, a, i);
}

// 1001 00DD DDDA AAAA IIII IIII IIII IIII l.lbs
void Assembler::l_lbs(reg_t d, reg_t a, int16 i) {
  AppendInstructionTypeDAI(0x24, d, a, i);
}

// 1001 01DD DDDA AAAA IIII IIII IIII IIII l.lhz
void Assembler::l_lhz(reg_t d, reg_t a, int16 i) {
  AppendInstructionTypeDAI(0x25, d, a, i);
}

// 1001 10DD DDDA AAAA IIII IIII IIII IIII l.lhs
void Assembler::l_lhs(reg_t d, reg_t a, int16 i) {
  AppendInstructionTypeDAI(0x26, d, a, i);
}

// 1001 11DD DDDA AAAA IIII IIII IIII IIII l.addi
void Assembler::l_addi(reg_t d, reg_t a, int16 i) {
  AppendInstructionTypeDAI(0x27, d, a, i);
}

// 1010 01DD DDDA AAAA KKKK KKKK KKKK KKKK l.andi
void Assembler::l_andi(reg_t d, reg_t a, uint16 k) {
  AppendInstructionTypeDAK(0x29, d, a, k);
}

// 1010 10DD DDDA AAAA KKKK KKKK KKKK KKKK l.ori
void Assembler::l_ori(reg_t d, reg_t a, uint16 k) {
  AppendInstructionTypeDAK(0x2a, d, a, k);
}

// 1010 11DD DDDA AAAA IIII IIII IIII IIII l.xori
void Assembler::l_xori(reg_t d, reg_t a, int16 i) {
  AppendInstructionTypeDAI(0x2b, d, a, i);
}

// 1011 01DD DDDA AAAA KKKK KKKK KKKK KKKK l.mfspr
void Assembler::l_mfspr(reg_t d, reg_t a, uint16 k) {
  AppendInstructionTypeDAK(0x2d, d, a, k);
}

// 1011 10DD DDDA AAAA ---- ---- 00LL LLLL l.slli
void Assembler::l_slli(reg_t d, reg_t a, uint8 l) {
  AppendInstructionTypeDAFL(0x2e, d, a, 0x00, l);
}

// 1011 10DD DDDA AAAA ---- ---- 01LL LLLL l.srli
void Assembler::l_srli(reg_t d, reg_t a, uint8 l) {
  AppendInstructionTypeDAFL(0x2e, d, a, 0x01, l);
}

// 1011 10DD DDDA AAAA ---- ---- 10LL LLLL l.srai
void Assembler::l_srai(reg_t d, reg_t a, uint8 l) {
  AppendInstructionTypeDAFL(0x2e, d, a, 0x02, l);
}

// 1011 1100 000A AAAA IIII IIII IIII IIII l.sfeqi
void Assembler::l_sfeqi(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5e0, a, i);
}

// 1011 1100 001A AAAA IIII IIII IIII IIII l.sfnei
void Assembler::l_sfnei(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5e1, a, i);
}

// 1011 1100 010A AAAA IIII IIII IIII IIII l.sfgtui
void Assembler::l_sfgtui(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5e2, a, i);
}

// 1011 1100 011A AAAA IIII IIII IIII IIII l.sfgeui
void Assembler::l_sfgeui(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5e3, a, i);
}

// 1011 1100 100A AAAA IIII IIII IIII IIII l.sfltui
void Assembler::l_sfltui(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5e4, a, i);
}

// 1011 1100 101A AAAA IIII IIII IIII IIII l.sfleui
void Assembler::l_sfleui(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5e5, a, i);
}

// 1011 1101 010A AAAA IIII IIII IIII IIII l.sfgtsi
void Assembler::l_sfgtsi(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5ea, a, i);
}

// 1011 1101 011A AAAA IIII IIII IIII IIII l.sfgesi
void Assembler::l_sfgesi(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5eb, a, i);
}

// 1011 1101 100A AAAA IIII IIII IIII IIII l.sfltsi
void Assembler::l_sfltsi(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5ec, a, i);
}

// 1011 1101 101A AAAA IIII IIII IIII IIII l.sflesi
void Assembler::l_sflesi(reg_t a, int16 i) {
  AppendInstructionTypeAI(0x5ed, a, i);
}

// 1100 00KK KKKA AAAA BBBB BKKK KKKK KKKK l.mtspr
void Assembler::l_mtspr(reg_t a, reg_t b, uint16 k) {
  AppendInstructionTypeABK(0x30, a, b, k);
}

// 1101 01II IIIA AAAA BBBB BIII IIII IIII l.sw
void Assembler::l_sw(reg_t a, reg_t b, int16 i) {
  AppendInstructionTypeABI(0x35, a, b, i);
}

// 1101 10II IIIA AAAA BBBB BIII IIII IIII l.sb
void Assembler::l_sb(reg_t a, reg_t b, int16 i) {
  AppendInstructionTypeABI(0x36, a, b, i);
}

// 1101 11II IIIA AAAA BBBB BIII IIII IIII l.sh
void Assembler::l_sh(reg_t a, reg_t b, int16 i) {
  AppendInstructionTypeABI(0x37, a, b, i);
}

// 1110 00DD DDDA AAAA BBBB B-00 ---- 0000 l.add
void Assembler::l_add(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x0, 0x0);
}

// 1110 00DD DDDA AAAA BBBB B-00 ---- 0010 l.sub
void Assembler::l_sub(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x0, 0x2);
}

// 1110 00DD DDDA AAAA BBBB B-00 ---- 0011 l.and
void Assembler::l_and(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x0, 0x3);
}

// 1110 00DD DDDA AAAA BBBB B-00 ---- 0100 l.or
void Assembler::l_or(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x0, 0x4);
}

// 1110 00DD DDDA AAAA BBBB B-00 ---- 0101 l.xor
void Assembler::l_xor(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x0, 0x5);
}

// 1110 00DD DDDA AAAA BBBB B-00 ---- 1111 l.ff1
void Assembler::l_ff1(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x0, 0xf);
}

// 1110 00DD DDDA AAAA BBBB B-00 00-- 1000 l.sll
void Assembler::l_sll(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x0, 0x8);
}

// 1110 00DD DDDA AAAA BBBB B-00 01-- 1000 l.srl
void Assembler::l_srl(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x1, 0x8);
}

// 1110 00DD DDDA AAAA BBBB B-00 10-- 1000 l.sra
void Assembler::l_sra(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x2, 0x8);
}

// 1110 00DD DDDA AAAA BBBB B-01 ---- 1111 l.fl1
void Assembler::l_fl1(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0x4, 0xf);
}

// 1110 00DD DDDA AAAA BBBB B-11 ---- 0110 l.mul
void Assembler::l_mul(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0xc, 0x6);
}

// 1110 00DD DDDA AAAA BBBB B-11 ---- 1001 l.div
void Assembler::l_div(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0xc, 0x9);
}

// 1110 00DD DDDA AAAA BBBB B-11 ---- 1010 l.divu
void Assembler::l_divu(reg_t d, reg_t a, reg_t b) {
  AppendInstructionTypeDABFF(0x38, d, a, b, 0xc, 0xa);
}

// 1110 0100 000A AAAA BBBB B--- ---- ---- l.sfeq
void Assembler::l_sfeq(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x720, a, b);
}

// 1110 0100 001A AAAA BBBB B--- ---- ---- l.sfne
void Assembler::l_sfne(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x721, a, b);
}

// 1110 0100 010A AAAA BBBB B--- ---- ---- l.sfgtu
void Assembler::l_sfgtu(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x722, a, b);
}

// 1110 0100 011A AAAA BBBB B--- ---- ---- l.sfgeu
void Assembler::l_sfgeu(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x723, a, b);
}

// 1110 0100 100A AAAA BBBB B--- ---- ---- l.sfltu
void Assembler::l_sfltu(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x724, a, b);
}

// 1110 0100 101A AAAA BBBB B--- ---- ---- l.sfleu
void Assembler::l_sfleu(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x725, a, b);
}

// 1110 0101 010A AAAA BBBB B--- ---- ---- l.sfgts
void Assembler::l_sfgts(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x72a, a, b);
}

// 1110 0101 011A AAAA BBBB B--- ---- ---- l.sfges
void Assembler::l_sfges(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x72b, a, b);
}

// 1110 0101 100A AAAA BBBB B--- ---- ---- l.sflts
void Assembler::l_sflts(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x72c, a, b);
}

// 1110 0101 101A AAAA BBBB B--- ---- ---- l.sfles
void Assembler::l_sfles(reg_t a, reg_t b) {
  AppendInstructionTypeAB(0x72d, a, b);
}

void Assembler::SetAddress(uint32 address) {
  while(Size() < address) {
    l_nop();
  }
}

void Assembler::Data(uint32 data) {
  AppendInstruction(data);
}


void Assembler::AppendInstructionTypeN(uint8 opcode, uint32 n) {
  AppendInstruction((opcode << 26) | (n & 0x3ffffff));
}

void Assembler::AppendInstructionTypeEXK(uint8 opcode, uint16 k) {
  AppendInstruction((opcode << 24) | k);
}

void Assembler::AppendInstructionTypeDFK(uint8 opcode, reg_t d, bool f, uint16 k) {
  AppendInstruction(
    (opcode << 26) |
    (d << 21) |
    ((f ? 1 : 0) << 16) |
    k);
}

void Assembler::AppendInstructionTypeK(uint16 opcode, uint16 k) {
  AppendInstruction((opcode << 16) | k);
}

void Assembler::AppendInstructionTypeX(uint8 opcode) {
  AppendInstruction(opcode << 26);
}

void Assembler::AppendInstructionTypeB(uint8 opcode, reg_t b) {
  AppendInstruction((opcode << 26) | ((b&kRegMask) << 11));
}

void Assembler::AppendInstructionTypeXAI(uint8 opcode, reg_t a, int16 i) {
  AppendInstruction(
    (opcode << 26) |
    ((a&kRegMask) << 16) |
    static_cast<uint16>(i));
}

void Assembler::AppendInstructionTypeDAI(uint8 opcode, reg_t d, reg_t a, int16 i) {
  AppendInstruction(
    (opcode << 26) |
    ((d&kRegMask) << 21) |
    ((a&kRegMask) << 16) |
    static_cast<uint16>(i));
}

void Assembler::AppendInstructionTypeDAK(uint8 opcode, reg_t d, reg_t a, uint16 k) {
  AppendInstruction(
    (opcode << 26) |
    ((d&kRegMask) << 21) |
    ((a&kRegMask) << 16) |
    k);
}

void Assembler::AppendInstructionTypeDAFL(uint8 opcode, reg_t d, reg_t a,
  uint8 f, uint8 l) {
  AppendInstruction(
    (opcode << 26) |
    ((d&kRegMask) << 21) |
    ((a&kRegMask) << 16) |
    ((f & 0x3) << 6) |
    (l & 0x3f));
}

// 1011 1101 101A AAAA IIII IIII IIII IIII
void Assembler::AppendInstructionTypeAI(uint16 opcode, reg_t a, int16 i) {
  AppendInstruction(
    ((opcode&0x7ff) << 21) |
    ((a&kRegMask) << 16) |
    static_cast<uint16>(i));
}

// 1100 00KK KKKA AAAA BBBB BKKK KKKK KKKK
void Assembler::AppendInstructionTypeABK(uint8 opcode, reg_t a, reg_t b, uint16 k) {
  const uint16 k_top = (k&0xf800) >> 11;
  const uint16 k_bottom = k&0x7ff;

  AppendInstruction(
    (opcode << 26) |
    (k_top<<21) |
    ((a&kRegMask) << 16) |
    ((b&kRegMask) << 11) |
    k_bottom);
}

// 1101 11II IIIA AAAA BBBB BIII IIII IIII
// IIII IIII IIII IIII
void Assembler::AppendInstructionTypeABI(uint8 opcode, reg_t a, reg_t b, int16 i) {
  const uint16 i_top = (i&0xf800) >> 11;
  const uint16 i_bottom = i&0x7ff;

  AppendInstruction(
    (opcode << 26) |
    (i_top<<21) |
    ((a&kRegMask) << 16) |
    ((b&kRegMask) << 11) |
    i_bottom);
}

// 1110 00DD DDDA AAAA BBBB B-00 00-- 1000
void Assembler::AppendInstructionTypeDABFF(uint8 opcode, reg_t d, reg_t a,
  reg_t b, uint8 f1, uint8 f2) {
  AppendInstruction(
    (opcode << 26) |
    ((d&kRegMask) << 21) |
    ((a&kRegMask) << 16) |
    ((b&kRegMask) << 11) |
    ((f1&0xf) << 6) |
    (f2&0xf));
}

void Assembler::AppendInstructionTypeAB(uint16 opcode, reg_t a, reg_t b) {
  AppendInstruction(
    ((opcode&0x7ff) << 21) |
    ((a&kRegMask) << 16) |
    ((b&kRegMask) << 11));
}

void Assembler::AppendInstruction(uint32 system_endian_instruction) {
  const uint32 instruction = B32ENDIANSWAP(system_endian_instruction);
  const uint8* ptr = reinterpret_cast<const uint8*>(&instruction);

  for (int i = 0; i < 4; i++) {
    instructions_.push_back(*ptr++);
  }
}

