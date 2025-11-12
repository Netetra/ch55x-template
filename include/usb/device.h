#pragma once

#include <stdint.h>
#include "../register.h"

#define USB_BUFFER_SIZE 64

typedef void (*handle_func_t)(uint8_t);

uint8_t __ep0_buffer[USB_BUFFER_SIZE + 1];
uint8_t* ep0_buffer = __ep0_buffer;
uint8_t __ep1_buffer[USB_BUFFER_SIZE + 1];
uint8_t* ep1_buffer = __ep1_buffer;
uint8_t __ep2_buffer[USB_BUFFER_SIZE + 1];
uint8_t* ep2_buffer = __ep2_buffer;
uint8_t __ep3_buffer[USB_BUFFER_SIZE + 1];
uint8_t* ep3_buffer = __ep3_buffer;

void usbd_stall(void) {
  UEP0_CTRL = ((1 << 7) | (1 << 6) | (0b11 << 2) | (0b11 << 0)) & 0xFF;
}

void usbd_bus_reset(void) {
  UEP0_CTRL = (0b00 << 2) | (0b10 << 2);
  USB_DEV_AD = 0x00;
}

void usbd_init(void) {
  IE_USB = 0;
  USB_CTRL = 0;
  if ((uint16_t)ep0_buffer & 1) {
    ep0_buffer++;
  }
  UEP0_DMA_H = (uint16_t)ep0_buffer >> 8;
  UEP0_DMA_L = (uint16_t)ep0_buffer & 0xFF;
  usbd_bus_reset();
  USB_DEV_AD = 0x00;
  UDEV_CTRL = (1 << 5) | (1 << 4);
  USB_CTRL = (1 << 5) | (1 << 3) | (1 << 0);
  UDEV_CTRL |= (1 << 0);
  USB_INT_FG = 0xFF;
  USB_INT_EN = (1 << 1) | (1 << 0); // transfer, bus reset
  IE_USB = 1;
}
