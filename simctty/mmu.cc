// simctty
// Copyright 2014 Tom Harwood

#include "simctty/mmu.h"

MMU::MMU(Bus* bus, enum Type type)
  :
    bus_(bus),
    ram_(bus_->GetRAM()),
    raw_(ram_->Raw()),
    verbose_(false),
    verbose_store_(false),
    type_(type),
    is_enabled_(false),
    authed_page_(1),
    authed_phy_(0),
    stats_fast_hit_(0),
    stats_fast_miss_(0) {
  Reset();
}

MMU::~MMU() {
}

void MMU::Reset() {
  for (size_t i = 0; i < kSetCount; i++) {
    match_reg_[i] = 0;
  }
  ClearFastAuthCache();
}

bool MMU::IsEnabled() const {
  return is_enabled_;
}

void MMU::SetIsEnabled(bool is_enabled) {
  authed_page_ = 1;
  is_enabled_ = is_enabled;
}

void MMU::SetVerbose(bool verbose) {
  verbose_ = verbose;
}

void MMU::SetVerboseStore(bool verbose) {
  verbose_store_ = verbose;
}

uint8 MMU::Load8(uint32 address, Exception* exception, bool is_sm) const {
  bool is_ram;
  const uint32 phy_address = MapAddress(address, exception, is_sm, false, &is_ram);

  if (*exception != kExceptionNone) {
    return 0;
  }

  if (is_ram) {
    const uint32 result = ram_->Load8(phy_address, exception);

    return result;
  }

  const BusDevice* device = bus_->GetDevice(phy_address);
  if (!device) {
    *exception = kExceptionBusError;
    return 0;
  }

  return device->Load8(phy_address, exception);
}

void MMU::Store8(uint32 address, uint8 value, Exception* exception, bool is_sm) {
  bool is_ram;
  const uint32 phy_address = MapAddress(address, exception, is_sm, true, &is_ram);

  if (*exception != kExceptionNone) {
    return;
  }

  if (is_ram) {
    ram_->Store8(phy_address, value, exception);
    return;
  }

  BusDevice* device = bus_->GetDevice(phy_address);
  if (!device) {
    *exception = kExceptionBusError;
    return;
  }

  device->Store8(phy_address, value, exception);
}

uint16 MMU::Load16(uint32 address, Exception* exception, bool is_sm) const {
  if ((address & 0x1) != 0) {
    *exception = kExceptionAlignment;
    return 0;
  }

  bool is_ram;
  const uint32 phy_address = MapAddress(address, exception, is_sm, false, &is_ram);

  if (*exception != kExceptionNone) {
    return 0;
  }

  if (is_ram) {
    const uint16 value = ram_->Load16(phy_address, exception);
    return value;
  }

  const BusDevice* device = bus_->GetDevice(phy_address);
  if (!device) {
    *exception = kExceptionBusError;
    return 0;
  }

  return device->Load16(phy_address, exception);
}

void MMU::Store16(uint32 address, uint16 value, Exception* exception, bool is_sm) {
  if ((address & 0x1) != 0) {
    *exception = kExceptionAlignment;
    return;
  }

  bool is_ram;
  const uint32 phy_address = MapAddress(address, exception, is_sm, true, &is_ram);

  if (*exception != kExceptionNone) {
    return;
  }

  if (address <= kMaxRamAddress) {
    ram_->Store16(phy_address, value, exception);
    return;
  }

  BusDevice* device = bus_->GetDevice(phy_address);
  if (!device) {
    *exception = kExceptionBusError;
    return;
  }

  device->Store16(phy_address, value, exception);
}

uint32 MMU::Load32(uint32 address, Exception* exception, bool is_sm) const {
  if ((address & 0xffffe000) == authed_page_) {
    const uint32* u32address = reinterpret_cast<const uint32*>(raw_ + authed_phy_ + (address&0x1fff));
    *exception = kExceptionNone;

    stats_fast_hit_++;
    return *u32address;
  } else {
    stats_fast_miss_++;
  }

  bool is_ram;
  const uint32 phy_address = MapAddress(address, exception, is_sm, false, &is_ram);

  if (*exception != kExceptionNone) {
    return 0;
  }

  if (is_ram) {
    const uint32* u32address = reinterpret_cast<const uint32*>(raw_ + phy_address);
    *exception = kExceptionNone;

    return *u32address;
  }

  const BusDevice* device = bus_->GetDevice(phy_address);
  if (!device) {
    *exception = kExceptionBusError;
    return 0;
  }

  return device->Load32(phy_address, exception);
}

void MMU::Store32(uint32 address, uint32 value, Exception* exception, bool is_sm) {
  if ((address & 0x3) != 0) {
    *exception = kExceptionAlignment;
    return;
  }

  bool is_ram;
  const uint32 phy_address = MapAddress(address, exception, is_sm, true, &is_ram);

  if (*exception != kExceptionNone) {
    return;
  }

  if (is_ram) {
    ram_->Store32(phy_address, value, exception);
    return;
  }

  BusDevice* device = bus_->GetDevice(phy_address);
  if (!device) {
    *exception = kExceptionBusError;
    return;
  }

  device->Store32(phy_address, value, exception);
}

bool MMU::SetReg(reg_t index, uint32 value) {
  ClearFastAuthCache();

  if (index == 0) {
    control_register_ = value;
  } else if (index >= kMatchRegOffset && index < kMatchRegOffset + kSetCount) {
    match_reg_[index - kMatchRegOffset] = value;
  } else if (index >= kTranslateRegOffset && index < kTranslateRegOffset + kSetCount) {
    translate_reg_[index - kTranslateRegOffset] = value;
  } else {
    fprintf(stderr, "!!!!!!!!!Unknown Set MMU index %d\n", index);
    exit(1);
  }

  return true;
}

uint32 MMU::Reg(reg_t index) const {
  if (index == 0) {
    return control_register_;
  } else if (index >= kMatchRegOffset && index < kMatchRegOffset + kSetCount) {
    return match_reg_[index - kMatchRegOffset];
  } else if (index >= kTranslateRegOffset && index < kTranslateRegOffset + kSetCount) {
    return translate_reg_[index - kTranslateRegOffset];
  } else {
    fprintf(stderr, "!!!!!!!!!Unknown MMU index %d\n", index);
    exit(1);
  }
}

uint32 MMU::MapAddress(uint32 address, Exception* exception, bool is_sm, bool is_write, bool* is_ram) const {
  if (!is_enabled_) {
    *exception = kExceptionNone;
    *is_ram = address <= kMaxRamAddress;
    return address;
  }

  const uint32 page = address >> 0xd;
  const uint32 set = (address >> 0xd) % kUsedSetCount;

  const uint32 mr = match_reg_[set];

  uint32 phy_address = address;

  if (mr & 0x1 && mr >> 0xd == page) {
    const uint32 tr = translate_reg_[set];

    const static uint32 kDataURE = 0x40;
    const static uint32 kDataUWE = 0x80;
    const static uint32 kDataSRE = 0x100;
    const static uint32 kDataSWE = 0x200;
    const static uint32 kInstructionUXE = 0x80;
    const static uint32 kInstructionSXE = 0x40;

    bool ok = false;

    if (type_ == kInstruction) {
      ok = tr & (kInstructionSXE | kInstructionUXE);
    } else {
      if (is_sm) {
        if (is_write) {
          ok = tr & (kDataSWE | kDataUWE);
        } else {
          ok = tr & (kDataSRE | kDataURE);
        }
      } else {
        if (is_write) {
          ok = tr & kDataUWE;
        } else {
          ok = tr & kDataURE;
        }
      }
    }

    if (ok) {
      phy_address = (tr & 0xffffe000) | (address & 0x1fff);
      *exception = kExceptionNone;

      if (phy_address <= kMaxRamAddress) {
        authed_page_ = address & 0xffffe000;
        authed_phy_ = tr & 0xffffe000;
      } else {
        authed_page_ = 1;
      }
      *is_ram = phy_address <= kMaxRamAddress;
    } else {
      *exception = type_ == kData ? kExceptionDataPageFault : kExceptionInstructionPageFault;
      *is_ram = false;
    }
  } else {
    *exception = type_ == kData ? kExceptionDTLBMiss : kExceptionITLBMiss;
    *is_ram = false;
  }

  return phy_address;
}

void MMU::Print() const {
  for (size_t i = 0; i < kSetCount; i++) {
    const uint32 mr = match_reg_[i];
    const uint32 tr = translate_reg_[i];
    if (!mr & 0x1) {
      continue;
    }

    fprintf(stderr, "MMU contents(%2ld) %#x => %#x SWE=%d SRE=%d UWE=%d URE=%d\n", i, mr&0xffffe000, tr&0xffffe00, tr&0x200, tr&0x100, tr&0x80, tr&40);
  }
}

void MMU::ClearFastAuthCache() {
  authed_page_ = 1;
}

void MMU::SetFastAuthCache(uint32 page, uint32 phy) {
  authed_page_ = page;
  authed_phy_ = phy;
}

uint64 MMU::StatsFastHitCount() const {
  return stats_fast_hit_;
}

uint64 MMU::StatsFastMissCount() const {
  return stats_fast_miss_;
}

