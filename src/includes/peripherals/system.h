#ifndef __CH559_SYSTEM_H__
#define __CH559_SYSTEM_H__

#include "../util.h"

inline void __enter_safe_mode() {
  SAFE_MOD_REG = 0x55;
  SAFE_MOD_REG = 0xAA;
}

inline void __exit_safe_mode() {
  SAFE_MOD_REG = 0xFF;
}

#endif