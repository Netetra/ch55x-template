#include <ch559.h>
#include <stdio.h>

int putchar(int c) {
  while (TI == 0) {}
  SBUF = c & 0xFF;
  TI = 0;
  return c;
}

int getchar() {
  while (RI == 0) {}
  RI = 0;
  return SBUF;
}

void main() {
  clock_init();
  uart0_init(115200, true);

  char buffer[256];
  uint16_t ptr = 0;

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }

    while (true) {
      char c = getchar();
      buffer[ptr] = c;
      ptr++;
      if (c == '\n') {
        buffer[ptr] = '\0';
        ptr = 0;
        break;
      }
    }
    printf("Receive: %s", buffer);
  }
}
