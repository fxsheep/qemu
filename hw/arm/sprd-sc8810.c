#include "qemu/osdep.h"
#include "qemu/units.h"
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
    object_initialize_child(obj, "intc", &s->intc, TYPE_SPRD_SC8810_INTC);

    object_initialize_child(obj, "gptimer", &s->gptimer, TYPE_SPRD_SC8810_GP_TIMER);

    object_initialize_child(obj, "systimer", &s->systimer, TYPE_SPRD_SC8810_SYS_TIMER);

    object_initialize_child(obj, "adi", &s->adi, TYPE_SPRD_SC8810_ADI);

    pl011_create(memmap[UART_0].base, 0, serial_hd(0));
}

static void sc8810_realize(DeviceState *dev, Error **errp)
{
    SC8810State *s = SPRD_SC8810(dev);
    SysBusDevice *sysbusdev;
    MemoryRegion *irom = g_new(MemoryRegion, 1);

    if (!qdev_realize(DEVICE(&s->cpu), NULL, errp)) {
        return;
    }
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->intc), errp)) {
        return;
    }
    sysbusdev = SYS_BUS_DEVICE(&s->intc);
    sysbus_mmio_map(sysbusdev, 0, memmap[INTC].base);
    sysbus_connect_irq(sysbusdev, 0,
                       qdev_get_gpio_in(DEVICE(&s->cpu), ARM_CPU_IRQ));
    sysbus_connect_irq(sysbusdev, 1,
                       qdev_get_gpio_in(DEVICE(&s->cpu), ARM_CPU_FIQ));
    qdev_pass_gpios(DEVICE(&s->intc), dev, NULL);

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->gptimer), errp)) {
        return;
    }
    sysbusdev = SYS_BUS_DEVICE(&s->gptimer);
    sysbus_mmio_map(sysbusdev, 0, memmap[GPT].base);
    sysbus_connect_irq(sysbusdev, 0, qdev_get_gpio_in(dev, 5));
    sysbus_connect_irq(sysbusdev, 1, qdev_get_gpio_in(dev, 6));
    sysbus_connect_irq(sysbusdev, 2, qdev_get_gpio_in(dev, 7));

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->systimer), errp)) {
        return;
    }
    sysbusdev = SYS_BUS_DEVICE(&s->systimer);
    sysbus_mmio_map(sysbusdev, 0, memmap[SYST].base);
    sysbus_connect_irq(sysbusdev, 0, qdev_get_gpio_in(dev, 17));

    sysbus_create_simple("l2x0", memmap[PL310].base, NULL);

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->adi), errp)) {
        return;
    }
    sysbusdev = SYS_BUS_DEVICE(&s->adi);
    sysbus_mmio_map(sysbusdev, 0, memmap[ADI].base);
    sysbus_connect_irq(sysbusdev, 0, qdev_get_gpio_in(dev, 25));

    memory_region_init_ram(&s->iram_0, NULL, "iram 0",
                            16 * KiB, &error_abort);
    memory_region_init_ram(&s->iram_1, NULL, "iram 1",
                            16 * KiB, &error_abort);
    memory_region_init_ram(&s->iram_2, NULL, "iram 2",
                            12 * KiB, &error_abort);
    memory_region_init_ram(&s->dpmem, NULL, "dpmem",
                            4 * KiB, &error_abort);
    memory_region_add_subregion(get_system_memory(), memmap[IRAM_0].base,
                                &s->iram_0);
    memory_region_add_subregion(get_system_memory(), memmap[IRAM_1].base,
                                &s->iram_1);
    memory_region_add_subregion(get_system_memory(), memmap[IRAM_2].base,
                                &s->iram_2);
    memory_region_add_subregion(get_system_memory(), memmap[DPMEM].base,
                                &s->dpmem);
    memory_region_init_rom(irom, NULL, "sc8810.irom", memmap[IROM_0].size,
                           &error_fatal);
    memory_region_add_subregion(get_system_memory(), memmap[IROM_0].base, irom);
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
