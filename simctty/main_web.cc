#include <stdio.h>

#include "simctty/system.h"

int main(int argc, char** argv) {
  System system;

  uint64 cycle_count = 0;
  const uint64 cycles_per_iteration = 20000000 / 1000;

  fprintf(stdout, "Running\n");

  if (!system.LoadImageFile("vmlinux.bin", 0x100)) {
    fprintf(stderr, "Unable to load image\n");
    return EXIT_FAILURE;
  }

  while (system.Run(cycles_per_iteration)) {
    cycle_count += cycles_per_iteration;

    while (system.GetUART()->CanRead()) {
      fprintf(stderr, "%c", system.GetUART()->Read());
    }
  }

  return 1;
}
