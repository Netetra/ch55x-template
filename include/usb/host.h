#pragma once

#include <string.h>
#include "../clock.h"
#include "../register.h"
#include "../util.h"
#include "descriptor.h"

#define USBH_BUFFER_MAX_SIZE 64

#define USBH_USE_HUB0 0b00000001
#define USBH_USE_HUB1 0b00000010

enum HubStatus {
  DEVICE_DISCONNECT, /* 接続されてない */
  DEVICE_CONNECTED,  /* 接続されてる(未設定) */
  DEVICE_READY,      /* 接続されてる(設定済み) */
  DEVICE_ERROR       /* 接続されてるが通信はしない */
};

struct Hub {
  enum HubStatus status;
  uint8_t address;
  bool is_full_speed;
};

struct UsbHost {
  uint8_t flags;
  struct Hub hub[2];
};

struct UsbHost __usb_host;

uint8_t __tx_buffer[USBH_BUFFER_MAX_SIZE + 1];
uint8_t* tx_buffer = __tx_buffer;
uint8_t __rx_buffer[USBH_BUFFER_MAX_SIZE + 1];
uint8_t* rx_buffer = __rx_buffer;

#define TRANSFER_BUFFER_SIZE 128
uint8_t transfer_buffer[TRANSFER_BUFFER_SIZE];

struct SetupRequest set_address_req = {
    SETUP_REQUEST_DIR_OUT | SETUP_REQUEST_TYPE_STANDARD | REQUEST_TARGET_DEVICE,
    SET_ADDRESS,
    0x0000,
    0x0000,
    0x0000,
};

struct SetupRequest get_device_desc_req = {
    SETUP_REQUEST_DIR_IN | SETUP_REQUEST_TYPE_STANDARD | REQUEST_TARGET_DEVICE,
    GET_DESCRIPTOR,
    DEVICE_DESCRIPTOR << 8,
    0x0000,
    0x0012,  // requesting descriptor size: can be modified
};

bool usbh_is_attach(uint8_t hub) {
  if (hub == 0) {
    return (USB_HUB_ST & bUHS_H0_ATTACH) != 0;
  } else if (hub == 1) {
    return (USB_HUB_ST & bUHS_H1_ATTACH) != 0;
  }
  return false;
}

void usbh_check_attach(uint8_t hub) {
  if (__usb_host.hub[hub].status == DEVICE_DISCONNECT && usbh_is_attach(hub)) {
    __usb_host.hub[hub].status = DEVICE_CONNECTED;
    DEBUG("hub%d: device connected\n", hub);
  }
}

void usbh_check_detach(uint8_t hub) {
  if (__usb_host.hub[hub].status != DEVICE_DISCONNECT && !usbh_is_attach(hub)) {
    __usb_host.hub[hub].status = DEVICE_DISCONNECT;
    DEBUG("hub%d: device disconnected\n", hub);
  }
}

void usbh_bus_reset(uint8_t hub) {
  USB_DEV_AD = 0x00;
  if (hub == 0) {
    UHUB0_CTRL = UHUB0_CTRL & ~bUH_LOW_SPEED | bUH_BUS_RESET;
    delay_ms(15);
    UHUB0_CTRL &= ~bUH_BUS_RESET;
  } else if (hub == 1) {
    UHUB1_CTRL = UHUB1_CTRL & ~bUH_LOW_SPEED | bUH_BUS_RESET;
    delay_ms(15);
    UHUB1_CTRL &= ~bUH_BUS_RESET;
  }
  delay_us(250);
  DEBUG("hub%d: bus reset\n", hub);
}

void usbh_disable_port(uint8_t hub) {
  if (hub == 0) {
    UHUB0_CTRL = 0x00;
  } else if (hub == 1) {
    UHUB1_CTRL = 0x00;
  }
  DEBUG("hub%d: port disabled\n", hub);
}

bool usbh_enable_port(uint8_t hub) {
	if (hub == 0 && usbh_is_attach(0)) {
		if ((UHUB0_CTRL & bUH_PORT_EN) == 0x00) {
			if (USB_HUB_ST & bUHS_DM_LEVEL) {
        __usb_host.hub[0].is_full_speed = false;
				UHUB0_CTRL |= bUH_LOW_SPEED;
			} else {
        __usb_host.hub[0].is_full_speed = true;
      }
		}
		UHUB0_CTRL |= bUH_PORT_EN;
    DEBUG("hub0: port enabled\n");
		return false;
	} else if (hub == 1 && usbh_is_attach(1)) {
		if ((UHUB1_CTRL & bUH_PORT_EN) == 0x00) {
			if (USB_HUB_ST & bUHS_HM_LEVEL) {
        __usb_host.hub[1].is_full_speed = false;
				UHUB1_CTRL |= bUH_LOW_SPEED;
			} else {
        __usb_host.hub[1].is_full_speed = true;
      }
		}
		UHUB1_CTRL |= bUH_PORT_EN;
    DEBUG("hub1: port enabled\n");
		return false;
	}
	return true;
}

void usbh_device_init(uint8_t hub) {
  usbh_bus_reset(hub);
  const uint8_t PORT_RETRY = 10;
  uint8_t port_retry;
  for (port_retry = 0; port_retry < PORT_RETRY; port_retry++) {
    if (!usbh_enable_port(hub)) { break; }
  }
  if (port_retry == PORT_RETRY) {
    usbh_disable_port(hub);
    __usb_host.hub[hub].status = DEVICE_ERROR;
    DEBUG("hub%d: could not enable port\n", hub);
    return;
  }

  // TODO: set address
  // TODO: get device descirptor

  __usb_host.hub[hub].status = DEVICE_READY;
  DEBUG("hub%d: device initialized\n", hub);
}

void usbh_poll(uint8_t hub) {
  if (hub != 0 && hub != 1) { return; }
  if (hub == 0 && (__usb_host.flags & USBH_USE_HUB0) == 0) { return; }
  if (hub == 1 && (__usb_host.flags & USBH_USE_HUB1) == 0) { return; }
  usbh_check_detach(hub);
  switch (__usb_host.hub[hub].status) {
    case DEVICE_DISCONNECT:
      usbh_check_attach(hub);
      break;
    case DEVICE_CONNECTED:
      usbh_device_init(hub);
      break;
    case DEVICE_READY:
    case DEVICE_ERROR:
      // nop
      break;
  }
}

struct UsbHost* usbh_init(uint8_t flags) {
  IE_USB = 0;
  __usb_host.flags = flags;
  // 16bit alignment
  if ((uint16_t)tx_buffer & 1) { tx_buffer++; }
  if ((uint16_t)rx_buffer & 1) { rx_buffer++; }
	USB_CTRL = bUC_HOST_MODE;
	USB_DEV_AD = 0x00;
	UH_EP_MOD = bUH_EP_TX_EN | bUH_EP_RX_EN ;
	UH_RX_DMA_L = (uint16_t)rx_buffer & 0xFF;
  UH_RX_DMA_H = (uint16_t)rx_buffer >> 8;
	UH_TX_DMA_L = (uint16_t)tx_buffer & 0xFF;
  UH_TX_DMA_H = (uint16_t)tx_buffer >> 8;
	UH_RX_CTRL = 0x00;
	UH_TX_CTRL = 0x00;
	USB_CTRL = bUC_HOST_MODE | bUC_INT_BUSY | bUC_DMA_EN;
  if (__usb_host.flags & USBH_USE_HUB0) {
    usbh_disable_port(0);
  } else {
    UHUB0_CTRL = bUH_RECV_DIS | bUH_DP_PD_DIS | bUH_DM_PD_DIS;
  }
  if (__usb_host.flags & USBH_USE_HUB1) {
    usbh_disable_port(1);
  } else {
    UHUB1_CTRL = bUH_RECV_DIS | bUH_DP_PD_DIS | bUH_DM_PD_DIS;
  }
	UH_SETUP = bUH_SOF_EN;
	USB_INT_FG = 0xFF;
	// USB_INT_EN = bUIE_TRANSFER | bUIE_DETECT;
  // IE_USB = 1;
  DEBUG("usb host initialized.\n");
  return &__usb_host;
}

// void usbh_interrupt(void) __interrupt(8) __using(1) {}