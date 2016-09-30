#include "simctty/system.h"
#include <stdio.h>

System sys;

extern "C" {
size_t sys_load_image(const uint8* data, int length, size_t start_pc) {
  return sys.LoadImage(data, length, start_pc);
}

bool sys_run() {
  const uint64 cycles_per_iteration = 20000000 / 1000;
  return sys.Run(cycles_per_iteration);
}

void sys_keypress(uint8 key) {
  sys.GetUART()->Keypress(key);
}

bool sys_can_read() {
  return sys.GetUART()->CanRead();
}

uint8 sys_read() {
  return sys.GetUART()->Read();
}
}

