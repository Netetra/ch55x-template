#pragma once

#include <stdint.h>
#include "../register.h"
#include "descriptor.h"

#define USB_BUFFER_MAX_SIZE 64

// UsbDevice.ep_flags
#define USE_EP1_OUT 0b00000001
#define USE_EP1_IN  0b00000010
#define USE_EP2_OUT 0b00000100
#define USE_EP2_IN  0b00001000
#define USE_EP3_OUT 0b00010000
#define USE_EP3_IN  0b00100000

struct UsbDevice {
  uint8_t ep_flags;
  void (*get_descriptor)(struct SetupRequest*, void**, uint8_t*);
  uint8_t address;
  uint8_t configuration_num;
  struct SetupRequest last_setup_req;
  void* ep0_sending_data_ptr;
  uint16_t ep0_sending_data_len;
};

struct UsbDevice __usb_device;

uint8_t __ep0_buffer[USB_BUFFER_MAX_SIZE + 1];
uint8_t* ep0_buffer = __ep0_buffer;
uint8_t __ep1_buffer[USB_BUFFER_MAX_SIZE + 1];
uint8_t* ep1_buffer = __ep1_buffer;
uint8_t __ep2_buffer[USB_BUFFER_MAX_SIZE + 1];
uint8_t* ep2_buffer = __ep2_buffer;
uint8_t __ep3_buffer[USB_BUFFER_MAX_SIZE + 1];
uint8_t* ep3_buffer = __ep3_buffer;

void usbd_bus_reset(void) {
  uint8_t use_ep1 = __usb_device.ep_flags & (USE_EP1_OUT | USE_EP1_IN);
  uint8_t use_ep2 = __usb_device.ep_flags & (USE_EP2_OUT | USE_EP2_IN);
  uint8_t use_ep3 = __usb_device.ep_flags & (USE_EP3_OUT | USE_EP3_IN);

  UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  if (use_ep1) {
    UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  }
  if (use_ep2) {
    UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  }
  if (use_ep3) {
    UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
  }
  USB_DEV_AD = 0x00;
  DEBUG("bus reset\n");
}

void usbd_stall(void) {
  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
  DEBUG("stall\n");
}

void ep0_send_first(void* ptr, uint16_t len) {
  uint8_t tx_len = (len <= USB_BUFFER_MAX_SIZE) ? len : USB_BUFFER_MAX_SIZE;
  memcpy(ep0_buffer, ptr, tx_len);
  UEP0_T_LEN = tx_len;
  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
  __usb_device.ep0_sending_data_ptr = ptr + tx_len;
  __usb_device.ep0_sending_data_len = len - tx_len;
}

void ep0_send_next(void) {
  void* ptr = __usb_device.ep0_sending_data_ptr;
  uint16_t len = __usb_device.ep0_sending_data_len;
  uint8_t tx_len = (len <= USB_BUFFER_MAX_SIZE) ? len : USB_BUFFER_MAX_SIZE;
  memcpy(ep0_buffer, ptr, tx_len);
  UEP0_T_LEN = tx_len;
  UEP0_CTRL ^= (bUEP_R_TOG | bUEP_T_TOG);
  __usb_device.ep0_sending_data_ptr = ptr + tx_len;
  __usb_device.ep0_sending_data_len = len - tx_len;
}

void ep0_out(void) {}

void ep0_in(void) {
  struct SetupRequest* last_setup_req = &(__usb_device.last_setup_req);
  switch (last_setup_req->bRequest) {
    case SET_ADDRESS:
      USB_DEV_AD = USB_DEV_AD & 0x80 | __usb_device.address;
      UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
      DEBUG("set address: %d\n", USB_DEV_AD);
      break;
    case GET_DESCRIPTOR:
      ep0_send_next();
      break;
  }
}

void ep0_setup(void) {
  UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_NAK | UEP_T_RES_NAK;

  uint8_t rx_len = USB_RX_LEN;
  if (rx_len != sizeof(struct SetupRequest)) { return; }
  struct SetupRequest* last_setup_req = &(__usb_device.last_setup_req);
  memcpy(last_setup_req, ep0_buffer, rx_len);

  // supported standard request only
  if ((last_setup_req->bRequestType & 0x60) != 0x00) {
    DEBUG("not supported standard request\n");
    return;
  }

  switch (last_setup_req->bRequest) {
    case SET_ADDRESS:
      __usb_device.address = last_setup_req->wValue & 0xFF;
      ep0_send_first(NULL, 0);
      break;
    case GET_DESCRIPTOR:
      void* desc_ptr;
      uint16_t desc_len;
      __usb_device.get_descriptor(last_setup_req, &desc_ptr, &desc_len);
      if (desc_ptr == NULL && desc_len == 0) {
        usbd_stall();
        DEBUG("requested unknown descriptor\n");
        return;
      }
      ep0_send_first(desc_ptr, desc_len);
      break;
    case GET_CONFIGURATION:
      void* num_ptr = &(__usb_device.configuration_num);
      ep0_send_first(num_ptr, 1);
      break;
    case SET_CONFIGURATION:
      __usb_device.configuration_num = last_setup_req->wValue & 0xFF;
      ep0_send_first(NULL, 0);
      DEBUG("set configuration %d\n", __usb_device.configuration_num);
      break;
  }
}

struct UsbDevice* usbd_init(
  uint8_t ep_flags,
  void (*get_descriptor)(struct SetupRequest*, void**, uint8_t*)
) {
  IE_USB = 0;
  USB_CTRL = 0;

  __usb_device.ep_flags = ep_flags;
  __usb_device.get_descriptor = get_descriptor;

  uint8_t use_ep1_out = __usb_device.ep_flags & USE_EP1_OUT;
  uint8_t use_ep1_in = __usb_device.ep_flags & USE_EP1_IN;
  uint8_t use_ep2_out = __usb_device.ep_flags & USE_EP2_OUT;
  uint8_t use_ep2_in = __usb_device.ep_flags & USE_EP2_IN;
  uint8_t use_ep3_out = __usb_device.ep_flags & USE_EP3_OUT;
  uint8_t use_ep3_in = __usb_device.ep_flags & USE_EP3_IN;

  // 16bit alignment
  if ((uint16_t)ep0_buffer & 1) { ep0_buffer++; }
  if ((uint16_t)ep1_buffer & 1) { ep1_buffer++; }
  if ((uint16_t)ep2_buffer & 1) { ep2_buffer++; }
  if ((uint16_t)ep3_buffer & 1) { ep3_buffer++; }

  UEP4_1_MOD = 0;
  UEP2_3_MOD = 0;
  if (use_ep1_out) {
    UEP4_1_MOD |= bUEP1_RX_EN;
    DEBUG("endpoint1 set out\n");
  } else if (use_ep1_in) {
    UEP4_1_MOD |= bUEP1_TX_EN;
    DEBUG("endpoint1 set in\n");
  }
  if (use_ep2_out) {
    UEP2_3_MOD |= bUEP2_RX_EN;
    DEBUG("endpoint2 set out\n");
  } else if (use_ep2_in) {
    UEP2_3_MOD |= bUEP2_TX_EN;
    DEBUG("endpoint2 set in\n");
  }
  if (use_ep3_out) {
    UEP2_3_MOD |= bUEP3_RX_EN;
    DEBUG("endpoint3 set out\n");
  } else if (use_ep3_in) {
    UEP2_3_MOD |= bUEP3_TX_EN;
    DEBUG("endpoint3 set in\n");
  }

  UEP0_DMA_H = (uint16_t)ep0_buffer >> 8;
  UEP0_DMA_L = (uint16_t)ep0_buffer & 0xFF;
  if (use_ep1_out || use_ep1_in) {
    UEP1_DMA_H = (uint16_t)ep1_buffer >> 8;
    UEP1_DMA_L = (uint16_t)ep1_buffer & 0xFF;
  }
  if (use_ep2_out || use_ep2_in) {
    UEP2_DMA_H = (uint16_t)ep2_buffer >> 8;
    UEP2_DMA_L = (uint16_t)ep2_buffer & 0xFF;
  }
  if (use_ep3_out || use_ep3_in) {
    UEP3_DMA_H = (uint16_t)ep3_buffer >> 8;
    UEP3_DMA_L = (uint16_t)ep3_buffer & 0xFF;
  }

  usbd_bus_reset();

  USB_DEV_AD = 0x00;
  UDEV_CTRL = bUD_DP_PD_DIS | bUD_DM_PD_DIS;
  USB_CTRL = UC_SYS_CTRL_PU | bUC_INT_BUSY | bUC_DMA_EN;
  UDEV_CTRL |= bUD_PORT_EN;
  USB_INT_FG = 0xFF;
  USB_INT_EN = bUIE_TRANSFER | bUIE_BUS_RST;
  IE_USB = 1;

  DEBUG("usb device initialized.\n");
  return &__usb_device;
}

void usb_interrupt(void) __interrupt(8) __using(1) {
  if (UIF_TRANSFER) {
    if (U_IS_NAK) { return; }
    uint8_t interrupt_status = USB_INT_ST;
    switch (interrupt_status & 0b00111111) {
      case TOKEN_OUT | 0:
        ep0_out();
        break;
      case TOKEN_IN | 0:
        ep0_in();
        break;
      case TOKEN_SETUP | 0:
        ep0_setup();
        break;
      case TOKEN_OUT | 1:
      case TOKEN_OUT | 2:
      case TOKEN_OUT | 3:
        break;
      case TOKEN_IN | 1:
      case TOKEN_IN | 2:
      case TOKEN_IN | 3:
        break;
    }
    UIF_TRANSFER = 0;
  } else if (UIF_BUS_RST) {
    usbd_bus_reset();
    UIF_TRANSFER = 0;
    UIF_BUS_RST = 0;
  }
}
