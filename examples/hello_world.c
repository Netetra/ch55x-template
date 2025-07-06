#include <stdint.h>
#include "includes/ch559.h"

void main() {
  SER1_LCR_REG |= 1 << 7;
  SER1_ADDR_REG = 1;
  SER1_FIFO_REG = 13;
  SER1_IER_REG = 0;
  SER1_LCR_REG &= ~(1 << 7);
  XBUS_AUX_REG |= 1 << 4;
  SER1_IER_REG |= 1 << 5;
  PORT_CFG_REG &= ~(1 << 2);
  PORT_CFG_REG |= (1 << (2 + 4));
  P2_DIR_REG |= 1 << 7;
  SER1_LCR_REG |= 0x3;

  char* message = "Hello World!\n";

  while (1) {
    for (uint8_t i = 0; i < 13; i++) {
      while (!(SER1_LSR_REG & (1 << 5)) ) {}
      SER1_FIFO_REG = message[i];
    }
    for (uint32_t i = 0; i < (100000UL); i++) {
      __asm__("nop");
    }
  }
}