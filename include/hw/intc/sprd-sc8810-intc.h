#ifndef SPRD_SC8810_INTC_H
#define SPRD_SC8810_INTC_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_SPRD_SC8810_INTC  "sprd-sc8810-intc"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810INTCState, SPRD_SC8810_INTC)

#define SPRD_SC8810_INTC_NR 32

struct SC8810INTCState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    qemu_irq parent_fiq;
    qemu_irq parent_irq;

    uint32_t irq_raw_sts;
    uint32_t irq_enable;
    uint32_t fiq_raw_sts;
    uint32_t fiq_enable;
};

#endif
