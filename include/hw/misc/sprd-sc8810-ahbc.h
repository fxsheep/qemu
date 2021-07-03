#ifndef SPRD_SC8810_AHBC_H
#define SPRD_SC8810_AHBC_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_SPRD_SC8810_AHBC "sprd-sc8810-ahbc"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810AHBCState, SPRD_SC8810_AHBC)

struct SC8810AHBCState {
    /*< private >*/
    SysBusDevice parent_obj;
    /*< public >*/
    MemoryRegion iomem;
    uint32_t ctrl5;
};

#endif
