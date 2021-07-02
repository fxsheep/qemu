#ifndef SPRD_SC8810_GP_TIMER_H
#define SPRD_SC8810_GP_TIMER_H

#include "hw/ptimer.h"
#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_SPRD_SC8810_GP_TIMER "sprd-sc8810-gptimer"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810GPTState, SPRD_SC8810_GP_TIMER)

typedef struct SC8810GPTPTimerContext {
    SC8810GPTState *state;
    int index;
} SC8810GPTPTimerContext;

struct SC8810GPTState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/
    MemoryRegion iomem;
    QEMUTimer *timer;
    qemu_irq irq[3];

    uint32_t freq_hz[3];
    uint32_t run[3];
    uint32_t mode[3]; //0:one-time, 1:period
    uint32_t raw_irq_status[3];
    uint32_t irq_enable[3];
    ptimer_state *ptimer[3];
    SC8810GPTPTimerContext ptimer_context[3];
};

#endif
