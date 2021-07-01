#ifndef SPRD_SC8810_SYS_TIMER_H
#define SPRD_SC8810_SYS_TIMER_H

#include "hw/ptimer.h"
#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_SPRD_SC8810_SYS_TIMER "sprd-sc8810-systimer"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810SYSTState, SPRD_SC8810_SYS_TIMER)

struct SC8810SYSTState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/
    MemoryRegion iomem;
    QEMUTimer *timer;
    qemu_irq irq;

    uint32_t alarm;
    uint64_t offset;
    uint32_t raw_irq_status;
    uint32_t irq_enable;
    uint32_t hit_time;
};

#endif
