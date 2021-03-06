#ifndef HW_ARM_SPRD_SC8810_H
#define HW_ARM_SPRD_SC8810_H

#include "qemu/error-report.h"
#include "hw/char/serial.h"
#include "hw/arm/boot.h"
#include "hw/intc/sprd-sc8810-intc.h"
#include "hw/timer/sprd-sc8810-systimer.h"
#include "hw/timer/sprd-sc8810-gptimer.h"
#include "hw/ssi/sprd-sc8810-adi.h"
#include "hw/misc/sprd-sc8810-ahbc.h"

#include "target/arm/cpu.h"
#include "qom/object.h"

enum {
    SDRAM_0,
    AHBC,
    IRAM_0,
    IRAM_1,
    IRAM_2,
    DPMEM,
    INTC,
    GPT,
    ADI,
    UART_0,
    SYST,
    PL310,
    SDRAM_1,
    SDRAM_2,
    IROM_0
};

static const MemMapEntry memmap[] = {
    [SDRAM_0] =              {          0, 0x10000000 },
    [AHBC   ] =              { 0x20900000,   0x100000 },
    [IRAM_0 ] =              { 0x40000000,     0x4000 },
    [IRAM_1 ] =              { 0x40004000,     0x4000 },
    [IRAM_2 ] =              { 0x40008000,     0x3000 },
    [DPMEM  ] =              { 0x50000000,     0x1000 },
    [INTC   ] =              { 0x80003000,     0x1000 },
    [GPT    ] =              { 0x81000000,     0x1000 },
    [ADI    ] =              { 0x82000000,     0x0040 },
    [UART_0 ] =              { 0x84000000,  0x1000000 },
    [SYST   ] =              { 0x87003000,     0x1000 },
    [PL310  ] =              { 0xA2002000,     0x1000 },
    [SDRAM_1] =              { 0xc0000000, 0x10000000 },
    [SDRAM_2] =              { 0xe0000000, 0x10000000 },
    [IROM_0 ] =              { 0xffff0000,     0x8000 },
};

#define TYPE_SPRD_SC8810 "sprd-sc8810"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810State, SPRD_SC8810)

struct SC8810State {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/

    ARMCPU cpu;
    SC8810INTCState intc;
    SC8810GPTState gptimer;
    SC8810SYSTState systimer;
    SC8810ADIState adi;
    SC8810AHBCState ahbc;
    MemoryRegion sdram_0;
    MemoryRegion sdram_1;
    MemoryRegion sdram_2;
    MemoryRegion iram_0;
    MemoryRegion iram_1;
    MemoryRegion iram_2;
    MemoryRegion dpmem;
    MemoryRegion irom;
};

#endif // HW_ARM_SPRD_SC8810_H
