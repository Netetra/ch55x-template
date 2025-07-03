#include <stdint.h>
#include "ch559.h"

void main() {
  PORT_CFG_REG = 0b00101101;
  P1_DIR_REG = 0b00000100;
  P1_PU_REG = 0b11111011;
  P1_REG = 0x00;

  while (1) {
    P1_REG = (!(P1_REG & 0b00000100)) << 2;

    for (uint32_t i = 0; i < (100000UL); i++) {
      __asm__("nop");
    }
  }
}