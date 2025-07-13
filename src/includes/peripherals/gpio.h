#ifndef __CH559_GPIO_H__
#define __CH559_GPIO_H__

void port1_init(bool high_drive, bool open_drain) {
  if (high_drive) {
    SET_BIT(PORT_CFG_REG, 5);
  } else {
    CLR_BIT(PORT_CFG_REG, 5);
  }

  if (open_drain) {
    SET_BIT(PORT_CFG_REG, 1);
  } else {
    CLR_BIT(PORT_CFG_REG, 1);
  }
}

void port1_pin_init(uint8_t pin, bool pull_up, bool enable_input, bool dir) {
  if (pin > 7) { return; }

  if (pull_up) {
    SET_BIT(P1_PU_REG, pin);
  } else {
    CLR_BIT(P1_PU_REG, pin);
  }

  if (enable_input) {
    SET_BIT(P1_IE_REG, pin);
  } else {
    CLR_BIT(P1_IE_REG, pin);
  }

  if (dir) {
    SET_BIT(P1_DIR_REG, pin);
  } else {
    CLR_BIT(P1_DIR_REG, pin);
  }
}

inline void port1_set_high(uint8_t pin) {
  SET_BIT(P1_REG, pin);
}

inline void port1_set_low(uint8_t pin) {
  CLR_BIT(P1_REG, pin);
}

inline bool port1_is_high(uint8_t pin) {
  return (P1_REG >> pin) & 1;
}

inline bool port1_is_low(uint8_t pin) {
  return !port1_is_high(pin);
}

#endif