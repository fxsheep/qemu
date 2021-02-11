#ifndef HW_ARM_SPRD_SC8810_H
#define HW_ARM_SPRD_SC8810_H

#include "qemu/error-report.h"
#include "hw/char/serial.h"
#include "hw/arm/boot.h"

#include "target/arm/cpu.h"
#include "qom/object.h"

#define TYPE_SPRD_SC8810 "sprd-sc8810"
OBJECT_DECLARE_SIMPLE_TYPE(SC8810State, SPRD_SC8810)

struct SC8810State {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/

    ARMCPU cpu;
};

#endif // HW_ARM_SPRD_SC8810_H
