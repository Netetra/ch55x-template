#include <stdint.h>
#include "includes/ch559.h"

void main() {
  PORT_CFG_REG = 0b01001011;
  P2_DIR_REG = 0b00100000;
  P2_PU_REG =  0b11011111;

  PWM_CYCLE_REG = 100;
  PWM_CTRL_REG = 0b00000100;

  while (1) {
    for (uint8_t i = 0; i < 255; i++) {
      PWM_DATA2_REG = i;
      for (uint16_t j = 0; j < 5000UL; j++) { __asm__("nop"); }
    }
    for (uint8_t i = 0; i < 255; i++) {
      PWM_DATA2_REG = 255 - i;
      for (uint16_t j = 0; j < 5000UL; j++) { __asm__("nop"); }
    }
  }
}