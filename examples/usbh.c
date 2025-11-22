#define USE_UART0_STDIO
// #define ENABLE_DEBUG_LOG

#include <stdio.h>
#include <ch559.h>
#include <usb/host.h>

uint8_t connected_handler(uint8_t hub, struct DeviceDesc* device_desc) {
  printf("hub%d: VID 0x%04X , PID 0x%04X\n", hub, device_desc->idVendor, device_desc->idProduct);
  return false;
}

void disconnected_handler(uint8_t hub) {
  printf("hub%d: disconnected\n", hub);
}

void poll_handler(uint8_t hub) {
  printf("hub%d: poll\n", hub);
}

void main(void) {
  clock_init();
  uart0_init(115200, true);
  uint8_t flags = USBH_USE_HUB0 | USBH_USE_HUB1;
  struct UsbHost* host = usbh_init(flags, connected_handler, disconnected_handler, poll_handler);
  // EA = 1;

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }
    usbh_poll(0);
    usbh_poll(1);
  }
}
