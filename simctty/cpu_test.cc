// simctty
// Copyright 2014 Tom Harwood

#include "gtest/gtest.h"

#include <stdio.h>

#include "simctty/assembler.h"
#include "simctty/cpu.h"
#include "simctty/system.h"

// Set Flag w/Intermediate instruction variant (e.g. l.sfgtui) testcases.
struct SFITestcase {
  int32 value;
  int16 intermediate;
  bool is_equal;
  bool is_gtui;
  bool is_geui;
  bool is_ltui;
  bool is_leui;
  bool is_gtsi;
  bool is_gesi;
  bool is_ltsi;
  bool is_lesi;
};

// Set Flag instruction variant (e.g. l.sfgtu) testcases.
struct SFTestcase {
  int32 a;
  int32 b;
  bool is_equal;
  bool is_gtu;
  bool is_geu;
  bool is_ltu;
  bool is_leu;
  bool is_gts;
  bool is_ges;
  bool is_lts;
  bool is_les;
};

struct RegOpTestcase {
  int32 a;
  int32 b;
  int32 result;
};

class CPUTest : public ::testing::Test {
 public:
  CPUTest()
    :
      system_(),
      cpu_(system_.GetCPU()) {
  }

  void Run(size_t cycles = 0x10) {
    system_.GetRAM()->LoadImage(asm_.Instructions(), asm_.Size());

    cpu_->Run(cycles);
  }

  void Reset() {
    cpu_->Reset();
    asm_.Clear();
  }

  uint32 GetInstruction(size_t instruction_number = 0) const {
    Exception exception;
    return system_.GetRAM()->Load32(instruction_number * 4, &exception);
  }

  void RunSetFlagIntermediate(const struct SFITestcase& t,
    Assembler::SFIFunction function) {
    Reset();

    asm_.l_lwz(kR1, kR0, 0x100);
    (asm_.*function)(kR1, t.intermediate);
    asm_.l_trap();

    asm_.SetAddress(0x100);
    asm_.Data(t.value);

    Run();
  }

  void RunRegOp(const struct RegOpTestcase& t,
    Assembler::RegOpFunction function) {
    Reset();

    asm_.l_lwz(kR1, kR0, 0x100);
    asm_.l_lwz(kR2, kR0, 0x104);
    (asm_.*function)(kR3, kR1, kR2);
    asm_.l_trap();

    asm_.SetAddress(0x100);
    asm_.Data(t.a);
    asm_.Data(t.b);

    Run();
  }

  void RunSetFlag(const struct SFTestcase& t,
    Assembler::SFFunction function) {
    Reset();

    asm_.l_lwz(kR1, kR0, 0x100);
    asm_.l_lwz(kR2, kR0, 0x104);
    (asm_.*function)(kR1, kR2);
    asm_.l_trap();

    asm_.SetAddress(0x100);
    asm_.Data(t.a);
    asm_.Data(t.b);

    Run();
  }

  Assembler asm_;
  System system_;
  CPU* cpu_;

  // Addresses of special registers.
  const uint16 kSpRegVR = 0<<11 | 0;      // Version register.
  const uint16 kSpRegSup = 0<<11 | 17;    // Supervisor register.
  const uint16 kSpRegEPCR0 = 0<<11 | 32;  // Exception PC register.
  const uint16 kSpRegTTCR = 10<<11 | 1;   // Tick Timer Counter register.

 private:
  DISALLOW_COPY_AND_ASSIGN(CPUTest);
};

TEST_F(CPUTest, InitialState) {
  // All registers initially zero.
  for (size_t i = 0; i < CPU::kRegCount; i++) {
    EXPECT_EQ(cpu_->Reg(i), 0U);
  }

  // Supervisor mode enabled.
  ASSERT_EQ(CPU::kSM, cpu_->SpReg(kSpRegSup) & CPU::kSM);
}

TEST_F(CPUTest, l_nop) {
  asm_.l_nop();
  asm_.l_nop(0xffff);
  asm_.l_nop(0xff00);
  asm_.l_nop(0x00ff);
  asm_.l_nop(0x12ef);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0x15000000U, GetInstruction(0));
  ASSERT_EQ(0x1500ffffU, GetInstruction(1));
  ASSERT_EQ(0x1500ff00U, GetInstruction(2));
  ASSERT_EQ(0x150000ffU, GetInstruction(3));
  ASSERT_EQ(0x150012efU, GetInstruction(4));
}

TEST_F(CPUTest, l_ori) {
  asm_.l_ori(kR1, kR0, 0U);
  asm_.l_ori(kR2, kR0, 0xffffU);
  asm_.l_ori(kR3, kR0, 0x1234U);
  asm_.l_ori(kR4, kR3, 0x1U);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0U, cpu_->Reg(0));

  ASSERT_EQ(0U, cpu_->Reg(1));
  ASSERT_EQ(0xffffU, cpu_->Reg(2));
  ASSERT_EQ(0x1234U, cpu_->Reg(3));
  ASSERT_EQ(0x1235U, cpu_->Reg(4));
}

TEST_F(CPUTest, l_andi) {
  asm_.l_ori(kR1, kR0, 0xffff);
  asm_.l_andi(kR2, kR1, 0x0);
  asm_.l_andi(kR3, kR1, 0xffff);
  asm_.l_andi(kR4, kR1, 0xabcd);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0U, cpu_->Reg(0));

  ASSERT_EQ(0xffffU, cpu_->Reg(1));
  ASSERT_EQ(0x0U, cpu_->Reg(2));
  ASSERT_EQ(0xffffU, cpu_->Reg(3));
  ASSERT_EQ(0xabcdU, cpu_->Reg(4));
}

TEST_F(CPUTest, l_addi) {
  asm_.l_ori(kR1, kR0, 0x1000);
  asm_.l_addi(kR2, kR1, 0x0);
  asm_.l_addi(kR3, kR1, 0x1);
  asm_.l_addi(kR4, kR1, -0x1);
  asm_.l_addi(kR5, kR1, -0x2000);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0U, cpu_->Reg(0));

  ASSERT_EQ(0x1000U, cpu_->Reg(1));
  ASSERT_EQ(0x1000U, cpu_->Reg(2));
  ASSERT_EQ(0x1001U, cpu_->Reg(3));
  ASSERT_EQ(0x0fffU, cpu_->Reg(4));
  ASSERT_EQ(-0x1000, static_cast<int32>(cpu_->Reg(5)));
}

TEST_F(CPUTest, l_j) {
  asm_.l_j(5);
  asm_.l_addi(kR1, kR0, 0x1);
  asm_.l_addi(kR2, kR0, 0x1);
  asm_.l_addi(kR3, kR0, 0x1);
  asm_.l_trap();
  asm_.l_j(-2);
  asm_.l_nop();

  Run();

  ASSERT_EQ(0U, cpu_->Reg(0));
  ASSERT_EQ(1U, cpu_->Reg(1));
  ASSERT_EQ(0U, cpu_->Reg(2));
  ASSERT_EQ(1U, cpu_->Reg(3));
}

TEST_F(CPUTest, l_jal) {
  asm_.l_jal(3);
  asm_.l_addi(kR1, kR0, 0x1);
  asm_.l_addi(kR2, kR0, 0x1);
  asm_.l_addi(kR3, kR0, 0x1);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0U, cpu_->Reg(0));
  ASSERT_EQ(1U, cpu_->Reg(1));
  ASSERT_EQ(0U, cpu_->Reg(2));
  ASSERT_EQ(1U, cpu_->Reg(3));

  // Link register contains address of instruction after the delay slot.
  ASSERT_EQ(8U, cpu_->Reg(9));
}

TEST_F(CPUTest, l_movhi) {
  asm_.l_movhi(kR1, 0x0000);
  asm_.l_movhi(kR2, 0xffff);
  asm_.l_movhi(kR3, 0xf0f0);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0x00000000U, cpu_->Reg(0));
  ASSERT_EQ(0x00000000U, cpu_->Reg(1));
  ASSERT_EQ(0xffff0000U, cpu_->Reg(2));
  ASSERT_EQ(0xf0f00000U, cpu_->Reg(3));
}

TEST_F(CPUTest, l_jr) {
  asm_.l_addi(kR1, kR0, 16);   // PC=0
  asm_.l_jr(kR1);              // PC=4
  asm_.l_addi(kR2, kR0, 0x1);  // PC=8
  asm_.l_addi(kR3, kR0, 0x1);  // PC=12
  asm_.l_addi(kR4, kR0, 0x1);  // PC=16
  asm_.l_trap();               // PC=20

  Run();

  ASSERT_EQ(0U, cpu_->Reg(0));
  ASSERT_EQ(16U, cpu_->Reg(1));
  ASSERT_EQ(1U, cpu_->Reg(2));
  ASSERT_EQ(0U, cpu_->Reg(3));
  ASSERT_EQ(1U, cpu_->Reg(4));
}

TEST_F(CPUTest, l_jalr) {
  asm_.l_addi(kR1, kR0, 16);   // PC=0
  asm_.l_jalr(kR1);            // PC=4
  asm_.l_addi(kR2, kR0, 0x1);  // PC=8
  asm_.l_addi(kR3, kR0, 0x1);  // PC=12
  asm_.l_addi(kR4, kR0, 0x1);  // PC=16
  asm_.l_trap();               // PC=20

  Run();

  ASSERT_EQ(0U, cpu_->Reg(0));
  ASSERT_EQ(16U, cpu_->Reg(1));
  ASSERT_EQ(1U, cpu_->Reg(2));
  ASSERT_EQ(0U, cpu_->Reg(3));
  ASSERT_EQ(1U, cpu_->Reg(4));

  // Link register contains address of instruction after the delay slot.
  ASSERT_EQ(12U, cpu_->Reg(9));
}

TEST_F(CPUTest, l_sys) {
  asm_.l_sys();

  asm_.SetAddress(exceptionHandlers[kExceptionSystemCall].pc);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0xc04U, cpu_->PC());
}

TEST_F(CPUTest, l_rfe) {
  asm_.l_sys();
  asm_.l_addi(kR1, kR0, 0x1);
  asm_.l_trap();

  asm_.SetAddress(exceptionHandlers[kExceptionSystemCall].pc);
  asm_.l_rfe();
  asm_.l_addi(kR2, kR0, 0x1);

  Run();

  ASSERT_EQ(1U, cpu_->Reg(1));
  ASSERT_EQ(0U, cpu_->Reg(2));
}

TEST_F(CPUTest, l_lwz) {
  const uint32 v1 = 0x12345678U;

  asm_.l_lwz(kR1, kR0, 0x100);
  asm_.l_trap();

  asm_.SetAddress(0x100);
  asm_.Data(v1);

  Run();

  ASSERT_EQ(v1, cpu_->Reg(1));
}

TEST_F(CPUTest, l_lbs) {
  asm_.l_lbs(kR1, kR0, 0x100);
  asm_.l_lbs(kR2, kR0, 0x101);
  asm_.l_lbs(kR3, kR0, 0x102);
  asm_.l_lbs(kR4, kR0, 0x103);
  asm_.l_trap();

  asm_.SetAddress(0x100);
  asm_.Data(0x001122ff);

  Run();

  ASSERT_EQ(0x00U, cpu_->Reg(1));
  ASSERT_EQ(0x11U, cpu_->Reg(2));
  ASSERT_EQ(0x22U, cpu_->Reg(3));
  ASSERT_EQ(-0x1U, cpu_->Reg(4));
}

TEST_F(CPUTest, l_lhz) {
  asm_.l_lhz(kR1, kR0, 0x100);
  asm_.l_lhz(kR2, kR0, 0x102);
  asm_.l_trap();

  asm_.SetAddress(0x100);
  asm_.Data(0x12345678);

  Run();

  ASSERT_EQ(0x1234U, cpu_->Reg(1));
  ASSERT_EQ(0x5678U, cpu_->Reg(2));
}

TEST_F(CPUTest, l_lhs) {
  asm_.l_lhs(kR1, kR0, 0x100);
  asm_.l_lhs(kR2, kR0, 0x102);
  asm_.l_trap();

  asm_.SetAddress(0x100);
  asm_.Data(0x1234fffe);

  Run();

  ASSERT_EQ(0x1234U, cpu_->Reg(1));
  ASSERT_EQ(-0x2U, cpu_->Reg(2));
}

TEST_F(CPUTest, l_xori) {
  asm_.l_addi(kR1, kR0, 0x1234);
  asm_.l_xori(kR2, kR1, 0x1235);

  asm_.l_trap();

  Run();

  ASSERT_EQ(1U, cpu_->Reg(2));
}

TEST_F(CPUTest, l_xori_signed) {
  asm_.l_lwz(kR1, kR0, 0x100);
  asm_.l_xori(kR2, kR1, -0x1);

  asm_.l_trap();

  asm_.SetAddress(0x100);
  asm_.Data(0xffffffff);

  Run();

  ASSERT_EQ(0xffffffffU, cpu_->Reg(1));
  ASSERT_EQ(0U, cpu_->Reg(3));
}

TEST_F(CPUTest, l_mfspr) {
  asm_.l_mfspr(kR1, kR0, kSpRegSup);
  asm_.l_trap();

  Run();

  ASSERT_EQ(cpu_->Reg(1), cpu_->SpReg(kSpRegSup));
  ASSERT_EQ(CPU::kSM, cpu_->SpReg(kSpRegSup) & CPU::kSM);
}

TEST_F(CPUTest, l_mtspr) {
  const uint32 magic = 0x12345678U;

  asm_.l_lwz(kR1, kR0, 0x100);
  asm_.l_mtspr(kR0, kR1, kSpRegEPCR0);
  asm_.l_trap();

  asm_.SetAddress(0x100);
  asm_.Data(magic);

  Run();

  ASSERT_EQ(magic, cpu_->SpReg(kSpRegEPCR0));
}

const struct SFITestcase sfi_tests[] = {
  {0, 0,             1, 0, 1, 0, 1, 0, 1, 0, 1},
  {1, 0,             0, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 1,             0, 0, 0, 1, 1, 0, 0, 1, 1},
  {0x7fff, 0x7fff,   1, 0, 1, 0, 1, 0, 1, 0, 1},
  {-0x7fff, -0x7fff, 1, 0, 1, 0, 1, 0, 1, 0, 1},
  {-0x1, -0x2,       0, 1, 1, 0, 0, 1, 1, 0, 0},
  {-0x2, -0x1,       0, 0, 0, 1, 1, 0, 0, 1, 1},
  {-0x1, 0x2,        0, 1, 1, 0, 0, 0, 0, 1, 1},
};

TEST_F(CPUTest, l_sfeqi) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfeqi);
    ASSERT_EQ(t.is_equal, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfnei) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfnei);
    ASSERT_EQ(!t.is_equal, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfgtui) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfgtui);
    ASSERT_EQ(t.is_gtui, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfgeui) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfgeui);
    ASSERT_EQ(t.is_geui, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfltui) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfltui);
    ASSERT_EQ(t.is_ltui, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfleui) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfleui);
    ASSERT_EQ(t.is_leui, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfgtsi) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfgtsi);
    ASSERT_EQ(t.is_gtsi, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfgesi) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfgesi);
    ASSERT_EQ(t.is_gesi, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfltsi) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sfltsi);
    ASSERT_EQ(t.is_ltsi, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sflesi) {
  for (SFITestcase t : sfi_tests) {
    RunSetFlagIntermediate(t, &Assembler::l_sflesi);
    ASSERT_EQ(t.is_lesi, cpu_->IsFlagSet());
  }
}

const struct SFTestcase sf_tests[] = {
  { 0x12345678,  0x12345678, 1, 0, 1, 0, 1, 0, 1, 0, 1},
  { 0x12345678, -0x12345678, 0, 0, 0, 1, 1, 1, 1, 0, 0},
  {-0x12345678,  0x12345678, 0, 1, 1, 0, 0, 0, 0, 1, 1},
  {-0x12345678, -0x12345678, 1, 0, 1, 0, 1, 0, 1, 0, 1},
  { 0x00000000,  0x00000000, 1, 0, 1, 0, 1, 0, 1, 0, 1},
  { 0x00000001,  0x00000002, 0, 0, 0, 1, 1, 0, 0, 1, 1},
  { 0x00000001, -0x00000002, 0, 0, 0, 1, 1, 1, 1, 0, 0},
  {-0x00000001,  0x00000002, 0, 1, 1, 0, 0, 0, 0, 1, 1},
  {-0x00000001, -0x00000002, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  { 0x00000012,  0x00000011, 0, 1, 1, 0, 0, 1, 1, 0, 0},
  { 0x00000012, -0x00000011, 0, 0, 0, 1, 1, 1, 1, 0, 0},
  {-0x00000012,  0x00000011, 0, 1, 1, 0, 0, 0, 0, 1, 1},
  {-0x00000012, -0x00000011, 0, 0, 0, 1, 1, 0, 0, 1, 1},
};

TEST_F(CPUTest, l_sfeq) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfeq);
    ASSERT_EQ(t.is_equal, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfne) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfne);
    ASSERT_EQ(!t.is_equal, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfgtu) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfgtu);
    ASSERT_EQ(t.is_gtu, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfgeu) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfgeu);
    ASSERT_EQ(t.is_geu, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfltu) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfltu);
    ASSERT_EQ(t.is_ltu, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfleu) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfleu);
    ASSERT_EQ(t.is_leu, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfgts) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfgts);
    ASSERT_EQ(t.is_gts, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfges) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfges);
    ASSERT_EQ(t.is_ges, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sflts) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sflts);
    ASSERT_EQ(t.is_lts, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_sfles) {
  for (SFTestcase t : sf_tests) {
    RunSetFlag(t, &Assembler::l_sfles);
    ASSERT_EQ(t.is_les, cpu_->IsFlagSet());
  }
}

TEST_F(CPUTest, l_slli) {
  const uint32 value = 0x12345678;

  for (uint8 i = 0; i < 32; i++) {
    Reset();

    asm_.l_lwz(kR1, kR0, 0x100);
    asm_.l_slli(kR2, kR1, i);
    asm_.l_trap();

    asm_.SetAddress(0x100);
    asm_.Data(value);

    Run();

    ASSERT_EQ(value << i, cpu_->Reg(2));
  }
}

TEST_F(CPUTest, l_srli) {
  const uint32 value = 0x12345678;

  for (uint8 i = 0; i < 32; i++) {
    Reset();

    asm_.l_lwz(kR1, kR0, 0x100);
    asm_.l_srli(kR2, kR1, i);
    asm_.l_trap();

    asm_.SetAddress(0x100);
    asm_.Data(value);

    Run();

    ASSERT_EQ(value >> i, cpu_->Reg(2));
  }
}

TEST_F(CPUTest, l_srai) {
  const int32 value = -0x1;

  for (uint8 i = 0; i < 32; i++) {
    Reset();

    asm_.l_lwz(kR1, kR0, 0x100);
    asm_.l_srai(kR2, kR1, i);
    asm_.l_trap();

    asm_.SetAddress(0x100);
    asm_.Data(value);

    Run();

    ASSERT_EQ(static_cast<uint32>(value >> i), cpu_->Reg(2));
    ASSERT_TRUE(cpu_->Reg(2) & 0x80000000);
  }
}

TEST_F(CPUTest, l_sw) {
  const int32 value = 0x12345678;

  for (int16 i = -32; i < 32; i++) {
    Reset();

    asm_.l_lwz(kR1, kR0, 0x100);   // value
    asm_.l_addi(kR2, kR1, i);      // value + i
    asm_.l_addi(kR3, kR0, 0x200);  // load/store base
    asm_.l_sw(kR3, kR2, i*4);
    asm_.l_lwz(kR4, kR3, i*4);
    asm_.l_trap();

    asm_.SetAddress(0x100);
    asm_.Data(value);

    Run();

    ASSERT_EQ(static_cast<uint32>(value + i), cpu_->Reg(4));
  }
}

TEST_F(CPUTest, l_sb) {
  for (uint8 i = 0; i < 128; i++) {
    Reset();

    asm_.l_addi(kR1, kR0, i);      // value
    asm_.l_addi(kR2, kR0, 0x200);  // load/store base
    asm_.l_sb(kR2, kR1, i);
    asm_.l_lbs(kR3, kR2, i);
    asm_.l_trap();

    Run();

    ASSERT_EQ(i, cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_sh) {
  int16 tests[] = {-0x7fff, -0x1, 0, 0x1, 0x7fff};

  for (int16 i : tests) {
    Reset();

    asm_.l_addi(kR1, kR0, i);      // value
    asm_.l_addi(kR2, kR0, 0x200);  // load/store base
    asm_.l_sh(kR2, kR1, 2);
    asm_.l_lhs(kR3, kR2, 2);
    asm_.l_trap();

    Run();

    ASSERT_EQ(static_cast<uint32>(i), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_add) {
  const RegOpTestcase tests[] = {
    {0, 0, 0},
    {1, 1, 2},
    {1, -1, 0},
    {-1, 1, 0},
    {-1, -1, -2},
    {0x7fffffff, 1, static_cast<int32>(0x80000000)},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_add);
    ASSERT_EQ(t.result, static_cast<int32>(cpu_->Reg(3)));
  }
}

TEST_F(CPUTest, l_sub) {
  const RegOpTestcase tests[] = {
    {0, 0, 0},
    {1, 1, 0},
    {1, -1, 2},
    {-1, 1, -2},
    {-1, -1, 0},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_sub);
    ASSERT_EQ(t.result, static_cast<int32>(cpu_->Reg(3)));
  }
}

TEST_F(CPUTest, l_or) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0x0},
    {0xf0, 0x0f, 0xff},
    {0x7, 0x8, 0xf},
    {0x0f0f0f0f, 0x00f0f0f0, 0x0fffffff},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_or);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_and) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0x0},
    {0xf0, 0x0f, 0x0},
    {0x1234, 0xf0f0, 0x1030},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_and);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_xor) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0x0},
    {0x1, 0x1, 0x0},
    {0x0, 0x1, 0x1},
    {0x0fffffff, 0x0fffffff, 0x0},
    {0x0f0f0f0f, 0x00f0f0f0, 0x0fffffff},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_xor);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_sll) {
  for (uint8 i = 0; i < 32; i++) {
    const RegOpTestcase t = {0x1, i, 0x1 << i};
    RunRegOp(t, &Assembler::l_sll);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_srl) {
  for (uint8 i = 0; i < 32; i++) {
    const RegOpTestcase t = {
      static_cast<int32>(0x80000000), i,
      static_cast<int32>(0x80000000>>i)};
    RunRegOp(t, &Assembler::l_srl);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_sra) {
  for (uint8 i = 0; i < 32; i++) {
    const RegOpTestcase t = {-1, i, -1 >> i};
    RunRegOp(t, &Assembler::l_sra);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_mul) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0x0},
    {0x1, 0x1, 0x1},
    {0x2, 0x2, 0x4},
    {-0x2, 0x2, -0x4},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_mul);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_div) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0x0},
    {0x1, 0x1, 0x1},
    {0x2, 0x2, 0x1},
    {0x4, 0x2, 0x2},
    {-0x4, -0x2, 0x2},
    {0x2, 0x0, 0x0},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_div);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_divu) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0x0},
    {0x1, 0x1, 0x1},
    {0x2, 0x2, 0x1},
    {0x4, 0x2, 0x2},
    {static_cast<int32>(0x80000000), 0x2, static_cast<int32>(0x40000000)},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_divu);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_ff1) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0},
    {0x1, 0x0, 1},
    {0x2, 0x0, 2},
    {0x4, 0x0, 3},
    {0x7fffffff, 0x0, 1},
    {0x40000000, 0x0, 31},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_ff1);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}

TEST_F(CPUTest, l_fl1) {
  const RegOpTestcase tests[] = {
    {0x0, 0x0, 0},
    {0x1, 0x0, 1},
    {0x2, 0x0, 2},
    {0x4, 0x0, 3},
    {0x7fffffff, 0x0, 31},
    {0x40000000, 0x0, 31},
    {0x00011000, 0x0, 17},
  };

  for (RegOpTestcase t : tests) {
    RunRegOp(t, &Assembler::l_fl1);
    ASSERT_EQ(static_cast<uint32>(t.result), cpu_->Reg(3));
  }
}
#
// Function that returns 42 (0x2a).
// 00000000 <f>:
//    0: d7 e1 17 fc   l.sw 0xfffffffc(r1),r2
//   4: 9c 41 00 00   l.addi r2,r1,0x0
//   8: 9c 21 ff fc   l.addi r1,r1,0xfffffffc
//   c: 9c 60 00 2a   l.addi r3,r0,0x2a
//  10: a9 63 00 00   l.ori r11,r3,0x0
//  14: a8 22 00 00   l.ori r1,r2,0x0
//  18: 84 41 ff fc   l.lwz r2,0xfffffffc(r1)
//  1c: 44 00 48 00   l.jr r9
//  20: 15 00 00 00   l.nop 0x0
TEST_F(CPUTest, ManualFunctionCall) {
  asm_.l_addi(kR1, kR0, 0xfe0);  // SP
  asm_.l_addi(kR2, kR0, 0xff0);  // FP
  asm_.l_addi(kR9, kR0, 0x100);  // Breakpoint address.
  asm_.Data(0xd7e117fc);
  asm_.Data(0x9c410000);
  asm_.Data(0x9c21fffc);
  asm_.Data(0x9c60002a);
  asm_.Data(0xa9630000);
  asm_.Data(0xa8220000);
  asm_.Data(0x8441fffc);
  asm_.Data(0x44004800);
  asm_.Data(0x15000000);

  Run();

  ASSERT_EQ(42U, cpu_->Reg(11));
}


// c00020f0:       18 a0 00 00     l.movhi r5,0x0
// c00020f4:       a8 a5 0a 00     l.ori r5,r5,0xa00
// c0002104:       c0 05 00 00     l.mtspr r5,r0,0x0
TEST_F(CPUTest, TLBRoutine) {
  asm_.Data(0x18a00000);
  asm_.Data(0xa8a50a00);
  asm_.Data(0xc0050000);
  asm_.l_trap();

  Run();

  ASSERT_EQ(0xa00U, cpu_->Reg(5));
}

TEST_F(CPUTest, 10MInstructions) {
  asm_.l_addi(kR1, kR1, 1);
  asm_.l_addi(kR1, kR1, 1);
  asm_.l_addi(kR1, kR1, 1);
  asm_.l_addi(kR1, kR1, 1);
  asm_.l_addi(kR1, kR1, 1);
  asm_.l_j(-5);
  asm_.l_addi(kR1, kR1, 1);

  Run(10000000);
  //ASSERT_EQ(10000000U, cpu_->InstructionRunCount());
}

