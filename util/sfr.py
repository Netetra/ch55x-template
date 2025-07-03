def generate(first_addr, last_addr, register_name):
    print("#ifndef __CH559_REGISTER_H__")
    print("#define __CH559_REGISTER_H__")
    print()

    print("#include <compiler.h>")
    print()

    for i in range(first_addr, last_addr + 1):
        try:
            name = register_name[i - first_addr] + "_REG"
        except:
            name = ""
        print(f"SFR({name}, 0x{i:X});")
        print()
    
    print("#endif")

def main():
    first_addr = 0x80
    last_addr = 0xFF
    register_name = [
        # 0x80
        "P0",
        "SP",
        "DPL",
        "DPH",
        "ROM_ADDR_L",
        "ROM_ADDR_H",
        "ROM_CTRL",
        "PCON",
        # 0x88
        "TCON",
        "TMOD",
        "TL0",
        "TL1",
        "TH0",
        "TH1",
        "ROM_DATA_L",
        "ROM_DATA_H",
        # 0x90
        "P1",
        "SER1_IER",
        "SER1_IIR",
        "SER1_LCR",
        "SER1_MCR",
        "SER1_LSR",
        "SER1_MSR",
        "SER1_ADDR",
        # 0x98
        "SCON",
        "SBUF",
        "SER1_FIFO",
        "PWM_DATA2",
        "PWM_DATA",
        "PWM_CTRL",
        "PWM_CK_SE",
        "PWM_CYCLE",
        # 0xA0
        "P2",
        "SAFE_MOD",
        "XBUS_AUX",
        "T3_SETUP",
        "T3_COUNT_L",
        "T3_COUNT_H",
        "T3_END_L",
        "T3_END_H",
        # 0xA8
        "IE",
        "T3_STAT",
        "T3_CTRL",
        "T3_DMA_CN",
        "T3_DMA_AL",
        "T3_DMA_AH",
        "T3_FIFO_L",
        "T3_FIFO_H",
        # 0xB0
        "P3",
        "GLOBAL_CFG",
        "PLL_CFG",
        "CLOCK_CFG",
        "SPI1_STAT",
        "SPI1_DATA",
        "SPI1_CTRL",
        "SPI1_CK_SE",
        # 0xB8
        "IP",
        "P1_IE",
        "P1_DIR",
        "P1_PU",
        "P2_DIR",
        "P2_PU",
        "P3_DIR",
        "P3_PU",
        # 0xC0
        "P4_OUT",
        "P4_IN",
        "P4_DIR",
        "P4_PU",
        "P0_DIR",
        "P0_PU",
        "PORT_CFG",
        "P5_PIN",
        # 0xC8
        "T2CON",
        "T2MOD",
        "RCAP2L",
        "RCAP2H",
        "TL2",
        "TH2",
        "PIN_FUNC",
        "GPIO_IE",
        # 0xD0
        "PSW",
        "USB_RX_LEN",
        "UEP1_CTRL",
        "UEP1_T_LEN",
        "UEP2_CTRL",
        "UEP2_T_LEN",
        "UEP3_CTRL",
        "UEP3_T_LEN",
        # 0xD8
        "USB_INT_FG",
        "USB_INT_ST",
        "USB_MIS_ST",
        "USB_HUB_ST",
        "UEP0_CTRL",
        "UEP0_T_LEN",
        "UEP4_CTRL",
        "UEP4_T_LEN",
        # 0xE0
        "ACC",
        "USB_INT_EN",
        "USB_CTRL",
        "USB_DEV_AD",
        "UDEV_CTRL",
        "UHUB1_CTRL",
        "USB_DMA_AL",
        "USB_DMA_AH",
        # 0xE8
        "IE_EX",
        "IP_EX",
        "SLEEP_CTRL",
        "WAKE_CTRL",
        "ADC_DMA_AL",
        "ADC_DMA_AH",
        "ADC_DMA_CN",
        "ADC_CK_SE",
        # 0xF0
        "B",
        "ADC_STAT",
        "ADC_CTRL",
        "ADC_CHANN",
        "ADC_FIFO_L",
        "ADC_FIFO_H",
        "ADC_SETUP",
        "ADC_EX_SW",
        # 0xF8
        "SPI0_STAT",
        "SPI0_DATA",
        "SPI0_CTRL",
        "SPI0_CK_SE",
        "SPI0_SETUP",
        "XBUS_SPEED",
        "RESET_KEEP",
        "WDOG_COUNT",
    ]

    generate(first_addr, last_addr, register_name)

main()