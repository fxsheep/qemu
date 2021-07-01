#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "hw/timer/sprd-sc8810-systimer.h"
#include "migration/vmstate.h"
#include "qemu/log.h"
#include "qemu/module.h"

enum {
    SYST_ALARM = 0x00,
    SYST_VALUE = 0x04,
    SYST_INT   = 0x08,
};

static uint32_t sprd_sc8810_systimer_get_value(void *opaque)
{
    SC8810SYSTState *s = opaque;
    return (qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) - s->offset) % 0x100000000;
}

static void sprd_sc8810_systimer_interrupt(void *opaque)
{
    SC8810SYSTState *s = opaque;

    s->raw_irq_status = 1;
    if (s->irq_enable) {
        qemu_irq_raise(s->irq);
    }
    timer_mod(s->timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + 0x100000000);
}

static uint64_t sprd_sc8810_systimer_read(void *opaque, hwaddr offset, unsigned size)
{
    SC8810SYSTState *s = SPRD_SC8810_SYS_TIMER(opaque);

    switch (offset) {
    case SYST_ALARM:
        return s->alarm;
    case SYST_VALUE:
        return sprd_sc8810_systimer_get_value(s);
    case SYST_INT:
        //syst_int_pls_sts unimplemented
        return s->irq_enable | s->raw_irq_status << 1 | (s->irq_enable & s->raw_irq_status) << 2;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
    return 0;
}

static void sprd_sc8810_systimer_write(void *opaque, hwaddr offset, uint64_t value,
                            unsigned size)
{
     SC8810SYSTState *s = SPRD_SC8810_SYS_TIMER(opaque);

    switch (offset) {
    case SYST_ALARM:
        s->alarm = value;
        if (s->alarm > sprd_sc8810_systimer_get_value(s)) {
            timer_mod(s->timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + s->alarm
             - sprd_sc8810_systimer_get_value(s));
        }
        else {
            timer_mod(s->timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + 0x100000000);
        }
        break;
    case SYST_VALUE:
        break;
    case SYST_INT:
        if (value & 8) {
            //Interrupt clear
            s->raw_irq_status = 0;
        }
        if (value & 1) {
            //Interrupt unmask
            s->irq_enable = 1;
        }
        else{
            //Interrupt mask
            s->irq_enable = 0;
        }
        if (!(s->raw_irq_status & s->irq_enable)) {
            qemu_irq_lower(s->irq);
        }
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
}

static const MemoryRegionOps sprd_sc8810_systimer_ops = {
    .read = sprd_sc8810_systimer_read,
    .write = sprd_sc8810_systimer_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_sprd_sc8810_systimer = {
    .name = "sc8810.systimer",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(alarm, SC8810SYSTState),
        VMSTATE_UINT64(offset, SC8810SYSTState),
        VMSTATE_UINT32(raw_irq_status, SC8810SYSTState),
        VMSTATE_UINT32(irq_enable, SC8810SYSTState),
        VMSTATE_UINT32(hit_time, SC8810SYSTState),
        VMSTATE_END_OF_LIST()
    }
};

static void sprd_sc8810_systimer_init(Object *obj)
{
    SC8810SYSTState *s = SPRD_SC8810_SYS_TIMER(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);

    sysbus_init_irq(dev, &s->irq);
    memory_region_init_io(&s->iomem, OBJECT(s), &sprd_sc8810_systimer_ops, s,
                           TYPE_SPRD_SC8810_SYS_TIMER, 0x1000);
    sysbus_init_mmio(dev, &s->iomem);
}

static void sprd_sc8810_systimer_reset(DeviceState *dev)
{
    SC8810SYSTState *s = SPRD_SC8810_SYS_TIMER(dev);
    s->alarm = 0xFFFF;
    s->raw_irq_status = 0;
    s->irq_enable = 0;
    s->offset = qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL);

    timer_mod(s->timer, qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL) + s->alarm);
}

static void sprd_sc8810_systimer_realize(DeviceState *dev, Error **errp)
{
    SC8810SYSTState *s = SPRD_SC8810_SYS_TIMER(dev);
    s->timer = timer_new_ms(QEMU_CLOCK_VIRTUAL, sprd_sc8810_systimer_interrupt, s);
}

static void sprd_sc8810_systimer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = sprd_sc8810_systimer_reset;
    dc->desc = "sprd sc8810 systimer";
    dc->vmsd = &vmstate_sprd_sc8810_systimer;
    dc->realize = sprd_sc8810_systimer_realize;
 }

static const TypeInfo sprd_sc8810_systimer_info = {
    .name = TYPE_SPRD_SC8810_SYS_TIMER,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SC8810SYSTState),
    .instance_init = sprd_sc8810_systimer_init,
    .class_init = sprd_sc8810_systimer_class_init,
};

static void sprd_sc8810_systimer_register_types(void)
{
    type_register_static(&sprd_sc8810_systimer_info);
}

type_init(sprd_sc8810_systimer_register_types);
