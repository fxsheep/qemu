#include "qemu/osdep.h"
#include "exec/address-spaces.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "cpu.h"
#include "hw/sysbus.h"
#include "hw/misc/unimp.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/char/serial.h"
#include "hw/char/pl011.h"
#include "hw/qdev-properties.h"
#include "hw/arm/sprd-sc8810.h"

static void sc8810_init(Object *obj)
{
    SC8810State *s = SPRD_SC8810(obj);

    object_initialize_child(obj, "cpu", &s->cpu,
                            ARM_CPU_TYPE_NAME("cortex-a8"));

    pl011_create(memmap[UART_0].base, 0, serial_hd(0));
}

static void sc8810_realize(DeviceState *dev, Error **errp)
{
    SC8810State *s = SPRD_SC8810(dev);

    if (!qdev_realize(DEVICE(&s->cpu), NULL, errp)) {
        return;
    }
}

static void sc8810_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = sc8810_realize;
    dc->user_creatable = false;
}

static const TypeInfo sprd_sc8810_type_info = {
    .name = TYPE_SPRD_SC8810,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(SC8810State),
    .instance_init = sc8810_init,
    .class_init = sc8810_class_init,
};

static void sprd_sc8810_register_types(void)
{
    type_register_static(&sprd_sc8810_type_info);
}

type_init(sprd_sc8810_register_types)
