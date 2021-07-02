#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "migration/vmstate.h"
#include "hw/intc/sprd-sc8810-intc.h"
#include "hw/irq.h"
#include "qemu/log.h"
#include "qemu/module.h"

enum {
    INT_IRQ_MASK_STS   = 0x00,
    INT_IRQ_RAW_STS    = 0x04,
    INT_IRQ_ENABLE     = 0x08,
    INT_IRQ_DISABLE    = 0x0C,
    INT_IRQ_SOFT       = 0x10,
    INT_IRQ_TEST_SRC   = 0x14,
    INT_IRQ_TEST_SEL   = 0x18,
    INT_FIQ_MASK_STS   = 0x20,
    INT_FIQ_RAW_STS    = 0x24,
    INT_FIQ_ENABLE     = 0x28,
    INT_FIQ_DISABLE    = 0x2C,
    INT_FIQ_SOFT       = 0x30,
    INT_FIQ_TEST_SRC   = 0x34,
    INT_FIQ_TEST_SEL   = 0x38,
};

static void sprd_sc8810_intc_update(SC8810INTCState *s)
{
    int irq, fiq;

    irq = s->irq_raw_sts & s->irq_enable;
    fiq = s->fiq_raw_sts & s->fiq_enable;

    qemu_set_irq(s->parent_irq, !!irq);
    qemu_set_irq(s->parent_fiq, !!fiq);
}

static void sprd_sc8810_intc_set_irq(void *opaque, int irq, int level)
{
    SC8810INTCState *s = opaque;

    if (level) {
        set_bit(irq, (void *)&s->irq_raw_sts);
        set_bit(irq, (void *)&s->fiq_raw_sts);
    } else {
        clear_bit(irq, (void *)&s->irq_raw_sts);
        clear_bit(irq, (void *)&s->fiq_raw_sts);
    }
    sprd_sc8810_intc_update(s);
}

static uint64_t sprd_sc8810_intc_read(void *opaque, hwaddr offset, unsigned size)
{
    SC8810INTCState *s = opaque;

    switch (offset) {
    case INT_IRQ_MASK_STS:
        return s->irq_raw_sts & s->irq_enable;
    case INT_IRQ_RAW_STS:
        return s->irq_raw_sts;
    case INT_IRQ_ENABLE:
        return s->irq_enable;
    case INT_IRQ_DISABLE:
        break;
    case INT_IRQ_SOFT:
        break;
    case INT_IRQ_TEST_SRC:
    /*   Unimplemented   */
        break;
    case INT_IRQ_TEST_SEL:
    /*   Unimplemented   */
        break;
    case INT_FIQ_MASK_STS:
        return s->fiq_raw_sts & s->fiq_enable;
    case INT_FIQ_RAW_STS:
        return s->fiq_raw_sts;
    case INT_FIQ_ENABLE:
        return s->fiq_enable;
    case INT_FIQ_DISABLE:
        break;
    case INT_FIQ_SOFT:
        break;
    case INT_FIQ_TEST_SRC:
    /*   Unimplemented   */
        break;
    case INT_FIQ_TEST_SEL:
    /*   Unimplemented   */
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }

    return 0;
}

static void sprd_sc8810_intc_write(void *opaque, hwaddr offset, uint64_t value,
                             unsigned size)
{
    SC8810INTCState *s = opaque;

    switch (offset) {
    case INT_IRQ_MASK_STS:
        break;
    case INT_IRQ_RAW_STS:
        break;
    case INT_IRQ_ENABLE:
        s->irq_enable |= value;
        break;
    case INT_IRQ_DISABLE:
        s->irq_enable &= ~value;
        break;
    case INT_IRQ_SOFT:
        s->irq_raw_sts = (s->irq_raw_sts & 0xFFFFFFFD) | (value & 2);
        break;
    case INT_IRQ_TEST_SRC:
    /*   Unimplemented   */
        break;
    case INT_IRQ_TEST_SEL:
    /*   Unimplemented   */
        break;
    case INT_FIQ_MASK_STS:
        break;
    case INT_FIQ_RAW_STS:
        break;
    case INT_FIQ_ENABLE:
        s->fiq_enable |= value;
        break;
    case INT_FIQ_DISABLE:
        s->fiq_enable &= ~value;
        break;
    case INT_FIQ_SOFT:
        s->fiq_raw_sts = (s->fiq_raw_sts & 0xFFFFFFFD) | (value & 2);
        break;
    case INT_FIQ_TEST_SRC:
    /*   Unimplemented   */
        break;
    case INT_FIQ_TEST_SEL:
    /*   Unimplemented   */
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }

    sprd_sc8810_intc_update(s);
}

static const MemoryRegionOps sprd_sc8810_intc_ops = {
    .read = sprd_sc8810_intc_read,
    .write = sprd_sc8810_intc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_sprd_sc8810_intc = {
    .name = "sc8810.intc",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(irq_raw_sts, SC8810INTCState),
        VMSTATE_UINT32(irq_enable, SC8810INTCState),
        VMSTATE_UINT32(fiq_raw_sts, SC8810INTCState),
        VMSTATE_UINT32(fiq_enable, SC8810INTCState),
        VMSTATE_END_OF_LIST()
    }
};

static void sprd_sc8810_intc_init(Object *obj)
{
    SC8810INTCState *s = SPRD_SC8810_INTC(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);

    qdev_init_gpio_in(DEVICE(dev), sprd_sc8810_intc_set_irq, SPRD_SC8810_INTC_NR);
    sysbus_init_irq(dev, &s->parent_irq);
    sysbus_init_irq(dev, &s->parent_fiq);
    memory_region_init_io(&s->iomem, OBJECT(s), &sprd_sc8810_intc_ops, s,
                          TYPE_SPRD_SC8810_INTC, 0x1000);
    sysbus_init_mmio(dev, &s->iomem);
}

static void sprd_sc8810_intc_reset(DeviceState *d)
{
    SC8810INTCState *s = SPRD_SC8810_INTC(d);

    s->irq_raw_sts = 0;
    s->irq_enable = 0;
    s->fiq_raw_sts = 0;
    s->fiq_enable = 0;
}

static void sprd_sc8810_intc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = sprd_sc8810_intc_reset;
    dc->desc = "sprd sc8810 intc";
    dc->vmsd = &vmstate_sprd_sc8810_intc;
 }

static const TypeInfo sprd_sc8810_intc_info = {
    .name = TYPE_SPRD_SC8810_INTC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SC8810INTCState),
    .instance_init = sprd_sc8810_intc_init,
    .class_init = sprd_sc8810_intc_class_init,
};

static void sprd_sc8810_intc_register_types(void)
{
    type_register_static(&sprd_sc8810_intc_info);
}

type_init(sprd_sc8810_intc_register_types);
