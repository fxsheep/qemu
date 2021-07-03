#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "hw/ssi/ssi.h"
#include "qemu/fifo32.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "migration/vmstate.h"
#include "hw/ssi/sprd-sc8810-adi.h"

/* ADI registers */
enum {
    ADI_CLK_DIV      = 0x00,
    ADI_CTRL0        = 0x04,
    ADI_CHNL_PRI     = 0x08,
    ADI_INT_EN       = 0x0C,
    ADI_INT_RAW_STS  = 0x10,
    ADI_INT_MASK_STS = 0x14,
    ADI_INT_CLR      = 0x18,
    ADI_RD_CMD       = 0x24,
    ADI_RD_DATA      = 0x28,
    ADI_FIFO_STS     = 0x2C,
    ADI_STS          = 0x30,
    ADI_REQ_STS      = 0x34,
};

static uint64_t sprd_sc8810_adi_read(void *opaque, hwaddr offset, unsigned size)
{
    SC8810ADIState *s = SPRD_SC8810_ADI(opaque);

    switch (offset) {
    case ADI_RD_DATA:
        return (s->addr & 0x7FF) << 16;
    case ADI_FIFO_STS:
        return 0x400;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
    return 0;
}

static void sprd_sc8810_adi_write(void *opaque, hwaddr offset, uint64_t value,
                            unsigned size)
{
     SC8810ADIState *s = SPRD_SC8810_ADI(opaque);

    switch (offset) {
    case ADI_RD_CMD:
        s->addr = value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
}

static const MemoryRegionOps sprd_sc8810_adi_ops = {
    .read = sprd_sc8810_adi_read,
    .write = sprd_sc8810_adi_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_sprd_sc8810_adi = {
    .name = "sc8810.adi",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_FIFO32(tx_fifo, SC8810ADIState),
        VMSTATE_FIFO32(rx_fifo, SC8810ADIState),
        VMSTATE_UINT32(addr, SC8810ADIState),
        VMSTATE_END_OF_LIST()
    }
};

static void sprd_sc8810_adi_reset(DeviceState *dev)
{
//    SC8810ADIState *s = SPRD_SC8810_ADI(dev);

}

static void sprd_sc8810_adi_realize(DeviceState *dev, Error **errp)
{
    SC8810ADIState *s = SPRD_SC8810_ADI(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    memory_region_init_io(&s->iomem, OBJECT(s), &sprd_sc8810_adi_ops, s,
                           TYPE_SPRD_SC8810_ADI, 0x40);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);

    s->bus = ssi_create_bus(dev, "adi");
    fifo32_create(&s->tx_fifo, ADI_FIFO_SIZE);
    fifo32_create(&s->rx_fifo, ADI_FIFO_SIZE);
}

static void sprd_sc8810_adi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = sprd_sc8810_adi_reset;
    dc->desc = "sprd sc8810 adi";
    dc->vmsd = &vmstate_sprd_sc8810_adi;
    dc->realize = sprd_sc8810_adi_realize;
 }

static const TypeInfo sprd_sc8810_adi_info = {
    .name = TYPE_SPRD_SC8810_ADI,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SC8810ADIState),
    .class_init = sprd_sc8810_adi_class_init,
};

static void sprd_sc8810_adi_register_types(void)
{
    type_register_static(&sprd_sc8810_adi_info);
}

type_init(sprd_sc8810_adi_register_types);
