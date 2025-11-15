#define USE_UART0_STDIO
#define ENABLE_DEBUG_LOG

#include <stdio.h>
#include <string.h>
#include <ch559.h>

struct DeviceDesc device_desc = {
  18       ,         // bLength
  DEVICE_DESCRIPTOR, // bDescriptorType: DEVICE
  0x0110,            // bcdUSB: USB 1.1
  0x00,              // bDeviceClass: 0
  0x00,              // bDeviceSubClass
  0x00,              // bDeviceProtocol
  64,                // bMaxPacketSize0: 64 bytes
  0x1a86,            // idVendor
  0x5678,            // idProduct
  0x0100,            // bcdDevice
  1,                 // iManufacturer
  2,                 // iProduct
  0,                 // iSerialNumber
  1                  // bNumConfigurations
};

struct ConfigurationDesc configuration_desc = {
  9,                        // bLength
  CONFIGURATION_DESCRIPTOR, // bDescriptorType: CONFIGURATION
  9,                        // wTotalLength
  0x00,                     // bNumInterfaces
  0x01,                     // bConfigurationValue
  0x00,                     // iConfiguration
  0x80,                     // bmAttributes
  0x32                      // bMaxPower: 100 mA
};

uint8_t string_desc0[] = { // LANGID
  0x06, 0x03,
  0x09, 0x04,
  0x11, 0x04
};

uint8_t string_desc1[] = { // iManufacturer
  24,  0x03,
  'n', 0x00,
  'e', 0x00,
  't', 0x00,
  'e', 0x00,
  't', 0x00,
  'r', 0x00,
  'a', 0x00,
  '.', 0x00,
  'd', 0x00,
  'e', 0x00,
  'v', 0x00,
};

uint8_t string_desc2[] = { // iProduct
  24,  0x03,
  't', 0x00,
  'e', 0x00,
  's', 0x00,
  't', 0x00,
  ' ', 0x00,
  'd', 0x00,
  'e', 0x00,
  'v', 0x00,
  'i', 0x00,
  'c', 0x00,
  'e', 0x00,
};

void get_descriptor(struct SetupRequest* last_setup_req, void** ptr, uint8_t* len) {
  uint8_t desc_type = (last_setup_req->wValue >> 8) & 0xFF;
  uint8_t index = last_setup_req->wValue & 0xFF;
  switch (desc_type) {
    case DEVICE_DESCRIPTOR:
      DEBUG("requested device descriptor\n");
      *ptr = &device_desc;
      *len = sizeof(struct DeviceDesc);
      break;
    case CONFIGURATION_DESCRIPTOR:
      DEBUG("requested configuration descriptor index: %d\n", index);
      switch (index) {
        case 0:
          *ptr = &configuration_desc;
          *len = sizeof(struct ConfigurationDesc);
          break;
        default:
          *ptr = NULL;
          *len = 0;
          break;
      }
      break;
    case STRING_DESCRIPTOR:
      DEBUG("requested string descriptor index: %d\n", index);
      switch (index) {
        case 0:
          *ptr = string_desc0;
          *len = sizeof(string_desc0);
          break;
        case 1:
          *ptr = string_desc1;
          *len = sizeof(string_desc1);
          break;
        case 2:
          *ptr = string_desc2;
          *len = sizeof(string_desc2);
          break;
        default:
          *ptr = NULL;
          *len = 0;
          break;
      }
      break;
    default:
      *ptr = NULL;
      *len = 0;
      break;
  }
}

void main(void) {
  clock_init();
  uart0_init(115200, true);
  uint8_t ep_flags = 0;
  struct UsbDevice* device = usbd_init(ep_flags, get_descriptor);
  EA = 1;

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }
    delay_ms(100);
  }
}
