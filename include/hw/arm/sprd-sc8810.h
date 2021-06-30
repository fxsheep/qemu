#ifndef HW_ARM_SPRD_SC8810_H
#define HW_ARM_SPRD_SC8810_H

#include "qemu/error-report.h"
#include "hw/char/serial.h"
#include "hw/arm/boot.h"
#include "hw/intc/sprd-sc8810-intc.h"

#include "target/arm/cpu.h"
#include "qom/object.h"

enum {
    SDRAM_0,
    IRAM_0,
    INTC,
    UART_0,
    PL310,
    SDRAM_1,
    SDRAM_2,
    IROM_0
};

static const MemMapEntry memmap[] = {
    [SDRAM_0] =              {          0, 0x10000000 },
    [IRAM_0 ] =              { 0x40000000, 0x10000000 },
    [INTC   ] =              { 0x80003000,     0x1000 },
    [UART_0 ] =              { 0x84000000,  0x1000000 },
    [PL310  ] =              { 0xA2002000,     0x1000 },
    [SDRAM_1] =              { 0xc0000000, 0x10000000 },
    [SDRAM_2] =              { 0xe0000000, 0x10000000 },
    [IROM_0 ] =              { 0xffff0000,    0x10000 },
};

#define TYPE_SPRD_SC8810 "sprd-sc8810"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810State, SPRD_SC8810)

struct SC8810State {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/

    ARMCPU cpu;
    SC8810INTCState intc;
};

#endif // HW_ARM_SPRD_SC8810_H
