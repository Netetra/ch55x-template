#define USE_UART0_STDIO
#define ENABLE_DEBUG_LOG

#include <stdio.h>
#include <ch559.h>
#include <usb/host.h>

__code struct SetupRequest set_configuration_req = {
  .bRequestType = SETUP_REQUEST_DIR_OUT,
  .bRequest = SET_CONFIGURATION,
  .wValue = 1,
  .wIndex = 0,
  .wLength = 0
};

uint8_t buffer[64];

uint8_t connected_handler(uint8_t hub, struct DeviceDesc* device_desc) {
  uint8_t error = usbh_transfer_control(hub, &set_configuration_req, 0, 0, 0);
  if (error) { return true; }
  DEBUG("hub%d: set configuration 1\n", hub);
  return false;
}

void disconnected_handler(uint8_t hub) {}

void poll_handler(uint8_t hub) {
  uint16_t len;
  uint8_t error = usbh_transfer_in(hub, 2, buffer, &len, 1);
  if (error) { return; }
  printf("hub%d: ", hub);
  for (uint8_t i = 0; i < len; i++) {
    printf("0x%02X  ", buffer[i]);
  }
  printf("\n");
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
