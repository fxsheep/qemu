#ifndef SPRD_SC8810_ADI_H
#define SPRD_SC8810_ADI_H

#include "hw/sysbus.h"
#include "qom/object.h"
#include "qemu/fifo32.h"

#define ADI_FIFO_SIZE 32

#define TYPE_SPRD_SC8810_ADI "sprd-sc8810-adi"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810ADIState, SPRD_SC8810_ADI)

struct SC8810ADIState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/
    MemoryRegion iomem;
    qemu_irq irq;
    SSIBus *bus;
    Fifo32 tx_fifo;
    Fifo32 rx_fifo;

    uint32_t addr;
};

#endif
