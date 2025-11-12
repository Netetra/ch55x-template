#include <ch559.h>
#include <stdio.h>
#include <string.h>

struct DeviceDesc device_desc = {
  18,         // bLength
  0x01,       // bDescriptorType: DEVICE
  0x0200,     // bcdUSB: USB 2.0
  0x00,       // bDeviceClass: 0
  0x00,       // bDeviceSubClass
  0x00,       // bDeviceProtocol
  64,         // bMaxPacketSize0: 64 bytes
  0x1a86,     // idVendor
  0x5678,     // idProduct
  0x0100,     // bcdDevice
  1,          // iManufacturer
  2,          // iProduct
  0x00,       // iSerialNumber
  1           // bNumConfigurations
};

struct ConfigurationDesc configuration_desc = {
  9,          // bLength
  0x02,       // bDescriptorType: CONFIGURATION
  9,          // wTotalLength
  0x01,       // bNumInterfaces
  0x01,       // bConfigurationValue
  0x00,       // iConfiguration
  0x80,       // bmAttributes
  0x32        // bMaxPower: 100 mA
};

uint8_t string_desc0[] = { // LANGID
  0x06,
  0x03,
  0x09, 0x04,
  0x11, 0x04
};

uint8_t string_desc1[] = { // iManufacturer
  24,
  0x03,
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
  24,
  0x03,
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

int putchar(int c) {
  while (TI == 0) {}
  SBUF = c & 0xFF;
  TI = 0;
  return c;
}

int getchar(void) {
  while (RI == 0) {}
  RI = 0;
  return SBUF;
}

static uint16_t setup_len;
static void* desc_ptr;
static uint8_t current_request_type;
static uint8_t usbd_addr;
static uint8_t current_configuration;

void ep0_handler(uint8_t token) {
  uint8_t rx_len = USB_RX_LEN;
  uint8_t rx_buf[64];
  switch (token) {
    case 0: // out
      switch (current_request_type) {
        case 0x06: // get descriptor
        default:
          UEP0_CTRL = (0b00 << 2) | (0b10 << 0);
      }
      break;
    case 2: // in
      switch (current_request_type) {
        case 0x05: // set address
          USB_DEV_AD = USB_DEV_AD & (1 << 7) | usbd_addr & 0xFF;
          UEP0_CTRL = (0b00 << 2) | (0b10 << 0);
          printf("device address: %d\n", USB_DEV_AD & 0x7F);
          break;
        case 0x06: // get descriptor
          uint8_t tx_len = setup_len >= USB_BUFFER_SIZE ? USB_BUFFER_SIZE : setup_len;
          memcpy(ep0_buffer, desc_ptr, tx_len);
          setup_len -= tx_len;
          desc_ptr += tx_len;
          UEP0_T_LEN = tx_len;
          UEP0_CTRL ^= 0x0b11000000;
          break;
        default:
          UEP0_T_LEN = 0;
          UEP0_CTRL = (0b00 << 2) | (0b10 << 0) & 0xFF;
          break;
      }
      break;
    case 3: // setup
      UEP0_CTRL = ((1 << 7) | (1 << 6) | (0b10 << 2) | (0b10 << 0)) & 0xFF;
      if (rx_len != sizeof(struct SetupRequest)) { return; }
      memcpy(rx_buf, ep0_buffer, rx_len);
      struct SetupRequest* setup_req = (struct SetupRequest*)rx_buf;
      current_request_type = setup_req->bRequest;
      if ((current_request_type & 0x60) != 0x00) { // supported standard request only
        printf("not supported standard request\n");
        return;
      }
      setup_len = setup_req->wLength;
      uint8_t tx_len;
      switch (current_request_type) {
        case 0x05: // set address
          usbd_addr = setup_req->wValue & 0xFF;
          tx_len = 0;
          break;
        case 0x06: // get descriptor
          switch ((setup_req->wValue >> 8) & 0xFF) {
            case 1: // device descriptor
              printf("sending device descriptor\n");
              desc_ptr = &device_desc;
              tx_len = 18;
              break;
            case 2: // configuration descriptor
              printf("sending configuration descriptor\n");
              desc_ptr = &configuration_desc;
              tx_len = 9;
              break;
            case 3: // string descriptor
              printf("sending string descriptor\n");
              switch (setup_req->wValue & 0xFF) {
                case 0:
                  desc_ptr = string_desc0;
                  tx_len = string_desc0[0];
                  break;
                case 1:
                  desc_ptr = string_desc1;
                  tx_len = string_desc1[0];
                  break;
                case 2:
                  desc_ptr = string_desc2;
                  tx_len = string_desc2[0];
                  break;
                default:
                  printf("requested unknown index string descriptor\n");
                  usbd_stall();
                  return;
              }
              break;
            default:
              printf("requested unknown descriptor.\n");
              usbd_stall();
              return;
          }
          break;
        case 0x08: // get configuration
          *ep0_buffer = current_configuration;
          if (setup_len >= 1) {
            tx_len = 1;
          }
        case 0x09: // set configuration
          current_configuration = setup_req->wValue & 0xFF;
          tx_len = 0;
          break;
      }
      if (setup_len > tx_len) {
        setup_len = tx_len;
      }
      tx_len = setup_len >= USB_BUFFER_SIZE ? USB_BUFFER_SIZE : setup_len;
      memcpy(ep0_buffer, desc_ptr, tx_len);
      setup_len -= tx_len;
      desc_ptr += tx_len;
      if (tx_len  <= USB_BUFFER_SIZE) {
        UEP0_T_LEN = tx_len;
      } else {
        UEP0_T_LEN = 0;
      }
      UEP0_CTRL = (1 << 7) | (1 << 6) | (0b00 << 2) | (0b00 << 0);
      break;
  }
}

void usb_interrupt(void) __interrupt(8) __using(1) {
  if (U_IS_NAK) { return; }
  if (UIF_TRANSFER) {
    uint8_t token = (USB_INT_ST & 0b00110000) >> 4;
    uint8_t ep_num = USB_INT_ST & 0b00001111;
    switch (ep_num) {
      case 0:
        ep0_handler(token);
        break;
    }
    UIF_TRANSFER = 0;
  } else if (UIF_BUS_RST) {
    printf("bus reset\n");
    usbd_bus_reset();
    UIF_TRANSFER = 0;
    UIF_BUS_RST = 0;
  }
}

void main(void) {
  clock_init();
  uart0_init(115200, true);
  usbd_init();
  EA = 1;

  printf("initialized.\n");

  while (true) {
    if(!(P4_IN & (1 << 6))) { run_bootloader(); }
    delay_ms(1000);
  }
}
