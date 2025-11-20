#define USE_UART0_STDIO
#define ENABLE_DEBUG_LOG

#include <stdio.h>
#include <ch559.h>
#include <usb/host.h>

void main(void) {
  clock_init();
  uart0_init(115200, true);
  uint8_t flags = USBH_USE_HUB0 | USBH_USE_HUB1;
  struct UsbHost* host = usbh_init(flags);
  EA = 1;

  while (true) {
    usbh_poll(0);
    usbh_poll(1);
  }
}
