#include "qemu/osdep.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "hw/misc/sprd-sc8810-ahbc.h"
#include "migration/vmstate.h"
#include "qemu/log.h"
#include "qemu/module.h"

enum {
    CTRL5 = 0x0218,
};

static uint64_t sprd_sc8810_ahbc_read(void *opaque, hwaddr offset, unsigned size)
{
    SC8810AHBCState *s = SPRD_SC8810_AHBC(opaque);

    switch (offset) {
    case CTRL5:
        return s->ctrl5;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
    return 0;
}

static void sprd_sc8810_ahbc_write(void *opaque, hwaddr offset, uint64_t value,
                            unsigned size)
{
     SC8810AHBCState *s = SPRD_SC8810_AHBC(opaque);

    switch (offset) {
    case CTRL5:
        s->ctrl5 = value;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
}

static const MemoryRegionOps sprd_sc8810_ahbc_ops = {
    .read = sprd_sc8810_ahbc_read,
    .write = sprd_sc8810_ahbc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_sprd_sc8810_ahbc = {
    .name = "sc8810.ahbc",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(ctrl5, SC8810AHBCState),
        VMSTATE_END_OF_LIST()
    }
};

static void sprd_sc8810_ahbc_init(Object *obj)
{
    SC8810AHBCState *s = SPRD_SC8810_AHBC(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &sprd_sc8810_ahbc_ops, s,
                           TYPE_SPRD_SC8810_AHBC, 0x100000);
    sysbus_init_mmio(dev, &s->iomem);
}

static void sprd_sc8810_ahbc_reset(DeviceState *dev)
{
    SC8810AHBCState *s = SPRD_SC8810_AHBC(dev);
    s->ctrl5 &= 0xFFFFFFFE;
}

static void sprd_sc8810_ahbc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = sprd_sc8810_ahbc_reset;
    dc->desc = "sprd sc8810 ahbc";
    dc->vmsd = &vmstate_sprd_sc8810_ahbc;
}

static const TypeInfo sprd_sc8810_ahbc_info = {
    .name = TYPE_SPRD_SC8810_AHBC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SC8810AHBCState),
    .instance_init = sprd_sc8810_ahbc_init,
    .class_init = sprd_sc8810_ahbc_class_init,
};

static void sprd_sc8810_ahbc_register_types(void)
{
    type_register_static(&sprd_sc8810_ahbc_info);
}

type_init(sprd_sc8810_ahbc_register_types);
