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

  printf("Chip: CH%03X\n", chip_id());

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }
    delay_ms(1000);
  }
}
