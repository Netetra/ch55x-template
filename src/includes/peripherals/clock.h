#ifndef __CH559_CLOCK_H__
#define __CH559_CLOCK_H__

#include "system.h"

void clock_init(uint8_t pll_mul, uint8_t fusb_div, uint8_t fsys_div) {
  uint8_t tmp = (fusb_div << 5) | pll_mul;
  
  __enter_safe_mode();
  PLL_CFG_REG = tmp;

  CLOCK_CFG_REG &= 0b11100000;
  CLOCK_CFG_REG |= fsys_div;
  __exit_safe_mode();
}

#endif