#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "hw/sysbus.h"
#include "hw/timer/sprd-sc8810-gptimer.h"
#include "migration/vmstate.h"
#include "qemu/log.h"
#include "qemu/module.h"

enum {
    TIMER0_LOAD  = 0x00,
    TIMER0_VALUE = 0x04,
    TIMER0_CTL   = 0x08,
    TIMER0_INT   = 0x0C,
    TIMER1_LOAD  = 0x20,
    TIMER1_VALUE = 0x24,
    TIMER1_CTL   = 0x28,
    TIMER1_INT   = 0x2C,
    TIMER2_LOAD  = 0x40,
    TIMER2_VALUE = 0x44,
    TIMER2_CTL   = 0x48,
    TIMER2_INT   = 0x4C,
};

static void sprd_sc8810_gptimer_update_irq(void *opaque)
{
    SC8810GPTState *s = opaque;
    int i;

    for (i = 0; i <= 2; i++) {
        qemu_set_irq(s->irq[i], s->raw_irq_status[i] & s->irq_enable[i]);
    }
}

static void sprd_sc8810_gptimer_interrupt(void *opaque)
{
    SC8810GPTPTimerContext *tc = opaque;
    SC8810GPTState *s = tc->state;
    int i = tc->index;

    if (s->run[i]) {
        s->raw_irq_status[i] = 1;
        if (!s->mode[i]) {
             ptimer_stop(s->ptimer[i]);
             s->run[i] = 0;
        }
        sprd_sc8810_gptimer_update_irq(s);
    }
}

static uint64_t sprd_sc8810_gptimer_read(void *opaque, hwaddr offset, unsigned size)
{
    SC8810GPTState *s = SPRD_SC8810_GP_TIMER(opaque);

    switch (offset) {
    case TIMER0_LOAD:
        return ptimer_get_limit(s->ptimer[0]);
    case TIMER0_VALUE:
        return ptimer_get_count(s->ptimer[0]);
    case TIMER0_CTL:
        return s->run[0] << 7 | s->mode[0] << 6;
    case TIMER0_INT:
        return s->irq_enable[0] | s->raw_irq_status[0] << 1 
                | (s->raw_irq_status[0] & s->irq_enable[0]) << 2;
    case TIMER1_LOAD:
        return ptimer_get_limit(s->ptimer[1]);
    case TIMER1_VALUE:
        return ptimer_get_count(s->ptimer[1]);
    case TIMER1_CTL:
        return s->run[1] << 7 | s->mode[1] << 6;
    case TIMER1_INT:
        return s->irq_enable[1] | s->raw_irq_status[1] << 1 
                | (s->raw_irq_status[1] & s->irq_enable[1]) << 2;
    case TIMER2_LOAD:
        return ptimer_get_limit(s->ptimer[2]);
    case TIMER2_VALUE:
        return ptimer_get_count(s->ptimer[2]);
    case TIMER2_CTL:
        return s->run[2] << 7 | s->mode[2] << 6;
    case TIMER2_INT:
        return s->irq_enable[2] | s->raw_irq_status[2] << 1 
                | (s->raw_irq_status[2] & s->irq_enable[2]) << 2;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
    return 0;
}

static void sprd_sc8810_gptimer_write(void *opaque, hwaddr offset, uint64_t value,
                            unsigned size)
{
     SC8810GPTState *s = SPRD_SC8810_GP_TIMER(opaque);

    switch (offset) {
    case TIMER0_LOAD:
        ptimer_transaction_begin(s->ptimer[0]);
        ptimer_set_limit(s->ptimer[0], value, 1);
        ptimer_transaction_commit(s->ptimer[0]);
        break;
    case TIMER0_VALUE:
        break;
    case TIMER0_CTL:
        s->run[0] = (value >> 7) & 1;
        s->mode[0] = (value >> 6) & 1;
        ptimer_transaction_begin(s->ptimer[0]);
        if (s->run[0]) {
            ptimer_run(s->ptimer[0], !s->mode[0]);
        }
        else {
            ptimer_stop(s->ptimer[0]);
        }
        ptimer_transaction_commit(s->ptimer[0]);
        break;
    case TIMER0_INT:
        if (value & 8) {
            //Interrupt clear
            s->raw_irq_status[0] = 0;
        }
        if (value & 1) {
            //Interrupt unmask
            s->irq_enable[0] = 1;
        }
        else {
            //Interrupt mask
            s->irq_enable[0] = 0;
        }
        sprd_sc8810_gptimer_update_irq(s);
        break;
    case TIMER1_LOAD:
        ptimer_transaction_begin(s->ptimer[1]);
        ptimer_set_limit(s->ptimer[1], value, 1);
        ptimer_transaction_commit(s->ptimer[1]);
        break;
    case TIMER1_VALUE:
        break;
    case TIMER1_CTL:
        s->run[1] = (value >> 7) & 1;
        s->mode[1] = (value >> 6) & 1;
        ptimer_transaction_begin(s->ptimer[1]);
        if (s->run[1]) {
            ptimer_run(s->ptimer[1], !s->mode[1]);
        }
        else {
            ptimer_stop(s->ptimer[1]);
        }
        ptimer_transaction_commit(s->ptimer[1]);
        break;
    case TIMER1_INT:
        if (value & 8) {
            //Interrupt clear
            s->raw_irq_status[1] = 0;
        }
        if (value & 1) {
            //Interrupt unmask
            s->irq_enable[1] = 1;
        }
        else {
            //Interrupt mask
            s->irq_enable[1] = 0;
        }
        sprd_sc8810_gptimer_update_irq(s);
        break;
    case TIMER2_LOAD:
        ptimer_transaction_begin(s->ptimer[2]);
        ptimer_set_limit(s->ptimer[2], value, 1);
        ptimer_transaction_commit(s->ptimer[2]);
        break;
    case TIMER2_VALUE:
        break;
    case TIMER2_CTL:
        s->run[2] = (value >> 7) & 1;
        s->mode[2] = (value >> 6) & 1;
        ptimer_transaction_begin(s->ptimer[2]);
        if (s->run[2]) {
            ptimer_run(s->ptimer[2], !s->mode[2]);
        }
        else {
            ptimer_stop(s->ptimer[2]);
        }
        ptimer_transaction_commit(s->ptimer[2]);
        break;
    case TIMER2_INT:
        if (value & 8) {
            //Interrupt clear
            s->raw_irq_status[2] = 0;
        }
        if (value & 1) {
            //Interrupt unmask
            s->irq_enable[2] = 1;
        }
        else {
            //Interrupt mask
            s->irq_enable[2] = 0;
        }
        sprd_sc8810_gptimer_update_irq(s);
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "%s: Bad offset 0x%x\n",  __func__, (int)offset);
        break;
    }
}

static const MemoryRegionOps sprd_sc8810_gptimer_ops = {
    .read = sprd_sc8810_gptimer_read,
    .write = sprd_sc8810_gptimer_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_sprd_sc8810_gptimer = {
    .name = "sc8810.gptimer",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(freq_hz, SC8810GPTState, 3),
        VMSTATE_UINT32_ARRAY(run, SC8810GPTState, 3),
        VMSTATE_UINT32_ARRAY(mode, SC8810GPTState, 3),
        VMSTATE_UINT32_ARRAY(raw_irq_status, SC8810GPTState, 3),
        VMSTATE_UINT32_ARRAY(irq_enable, SC8810GPTState, 3),
        VMSTATE_PTIMER_ARRAY(ptimer, SC8810GPTState, 3),
        VMSTATE_END_OF_LIST()
    }
};

static void sprd_sc8810_gptimer_init(Object *obj)
{
    SC8810GPTState *s = SPRD_SC8810_GP_TIMER(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);
    uint32_t i;

    for (i = 0; i <= 2; i++) {
        sysbus_init_irq(dev, &s->irq[i]);
    }
    memory_region_init_io(&s->iomem, OBJECT(s), &sprd_sc8810_gptimer_ops, s,
                           TYPE_SPRD_SC8810_GP_TIMER, 0x1000);
    sysbus_init_mmio(dev, &s->iomem);
}

static void sprd_sc8810_gptimer_reset(DeviceState *dev)
{
    SC8810GPTState *s = SPRD_SC8810_GP_TIMER(dev);
    uint32_t i;

    for (i = 0; i <= 2; i++) {
        ptimer_transaction_begin(s->ptimer[i]);
        ptimer_stop(s->ptimer[i]);
        ptimer_set_limit(s->ptimer[i], 0, 0);
        ptimer_transaction_commit(s->ptimer[i]);
    }
}

static void sprd_sc8810_gptimer_realize(DeviceState *dev, Error **errp)
{
    SC8810GPTState *s = SPRD_SC8810_GP_TIMER(dev);
    uint32_t i;

    s->freq_hz[0] = 32768;
    s->freq_hz[1] = 32768;
    s->freq_hz[2] = 26000000;

    for (i = 0; i <= 2; i++) {
        SC8810GPTPTimerContext *tc = &s->ptimer_context[i];
        tc->state = s;
        tc->index = i;

        s->ptimer[i] = ptimer_init(sprd_sc8810_gptimer_interrupt, tc, PTIMER_POLICY_DEFAULT);
        ptimer_transaction_begin(s->ptimer[i]);
        ptimer_set_freq(s->ptimer[i], s->freq_hz[i]);
        ptimer_transaction_commit(s->ptimer[i]);
    }
}

static void sprd_sc8810_gptimer_finalize(Object *obj)
{
    SC8810GPTState *s = SPRD_SC8810_GP_TIMER(obj);
    uint32_t i;

    for (i = 0; i <= 2; i++) {
        ptimer_free(s->ptimer[i]);
    }
}

static void sprd_sc8810_gptimer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = sprd_sc8810_gptimer_reset;
    dc->desc = "sprd sc8810 gptimer";
    dc->vmsd = &vmstate_sprd_sc8810_gptimer;
    dc->realize = sprd_sc8810_gptimer_realize;
 }

static const TypeInfo sprd_sc8810_gptimer_info = {
    .name = TYPE_SPRD_SC8810_GP_TIMER,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(SC8810GPTState),
    .instance_init = sprd_sc8810_gptimer_init,
    .instance_finalize = sprd_sc8810_gptimer_finalize,
    .class_init = sprd_sc8810_gptimer_class_init,
};

static void sprd_sc8810_gptimer_register_types(void)
{
    type_register_static(&sprd_sc8810_gptimer_info);
}

type_init(sprd_sc8810_gptimer_register_types);
