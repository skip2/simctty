// simctty
// Copyright 2014 Tom Harwood

#ifndef SIMCTTY_MMU_H_
#define SIMCTTY_MMU_H_

#include <stdio.h>

#include <string>
#include <vector>

#include "simctty/ram.h"
#include "simctty/bus_device.h"
#include "simctty/bus.h"
#include "simctty/exception.h"
#include "simctty/types.h"

class MMU {
 public:
  enum Type {
    kData,
    kInstruction,
  };

  MMU(Bus* bus, enum Type type);
  ~MMU();

  void Reset();

  void SetVerbose(bool verbose);
  void SetVerboseStore(bool verbose);

  bool IsEnabled() const;
  void SetIsEnabled(bool is_enabled);

  uint8 Load8(uint32 address, Exception* exception, bool is_sm) const;
  uint16 Load16(uint32 address, Exception* exception, bool is_sm) const;
  uint32 Load32(uint32 address, Exception* exception, bool is_sm) const;

  void Store8(uint32 address, uint8 value, Exception* exception, bool is_sm);
  void Store16(uint32 address, uint16 value, Exception* exception, bool is_sm);
  void Store32(uint32 address, uint32 value, Exception* exception, bool is_sm);

  bool SetReg(reg_t index, uint32 value);
  uint32 Reg(reg_t index) const;

  void Print() const;

  void ClearFastAuthCache();

  uint64 StatsFastHitCount() const;
  uint64 StatsFastMissCount() const;

  uint32 MapAddress(uint32 address, Exception* exception, bool is_sm, bool is_write, bool* is_ram) const;

 private:
  Bus* bus_;
  RAM* ram_;
  uint8* raw_;

  bool verbose_;
  bool verbose_store_;

  const Type type_;
  bool is_enabled_;

  uint32 control_register_;

  const static reg_t kMatchRegOffset = 512;
  const static reg_t kTranslateRegOffset = 640;

  const static uint32 kSetCount = 128;
  const static uint32 kUsedSetCount = 64;
  uint32 match_reg_[kSetCount];
  uint32 translate_reg_[kSetCount];

  bool VerboseLoadStore(const char* type, uint32 ea, uint32 phy, uint32 val) const;

  mutable uint32 authed_page_;
  mutable uint32 authed_phy_;
  void SetFastAuthCache(uint32 page, uint32 phy);

  mutable uint64 stats_fast_hit_;
  mutable uint64 stats_fast_miss_;

  DISALLOW_COPY_AND_ASSIGN(MMU);
};  // MMU

#endif  // SIMCTTY_MMU_H_

