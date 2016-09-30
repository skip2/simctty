#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "simctty/system.h"

bool keychar(uint8* key) {
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  if (FD_ISSET(0, &fds)) {
    const bool ok = read(STDIN_FILENO, key, 1) == 1;
    return ok;
  }

  return false;
}

int main(int argc, char** argv) {
  const char* default_filename = "vmlinux.bin";
  const char* filename;

  if (argc >= 2) {
    filename = argv[1];
  } else {
    filename = default_filename;
  }

  System system;

  if (!system.LoadImageFile(filename, 0x100)) {
    fprintf(stderr, "Unable to load image\n");
    return EXIT_FAILURE;
  }

  struct termios termios_p;
  tcgetattr(fileno(stdin), &termios_p);
  struct termios termios_p_saved = termios_p;

  termios_p.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                 | INLCR | IGNCR | ICRNL | IXON);
  termios_p.c_oflag &= ~OPOST;
  termios_p.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  termios_p.c_cflag &= ~(CSIZE | PARENB);
  termios_p.c_cflag |= CS8;
  tcsetattr(fileno(stdin), 0, &termios_p);

  uint64 cycle_count = 0;
  const uint64 cycles_per_iteration = 20000000 / 1000;
  while (system.Run(cycles_per_iteration)) {

    cycle_count += cycles_per_iteration;

    uint8 key;
    while (keychar(&key)) {
      system.GetUART()->Keypress(key);
    }

    while (system.GetUART()->CanRead()) {
      fprintf(stderr, "%c", system.GetUART()->Read());
    }
  }

  tcsetattr(fileno(stdin), 0, &termios_p_saved);

  return EXIT_SUCCESS;
}

