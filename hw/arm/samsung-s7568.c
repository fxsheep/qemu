#include "qemu/osdep.h"
#include "exec/address-spaces.h"
#include "qapi/error.h"
#include "qemu/error-report.h"
#include "qemu/module.h"
#include "qemu/units.h"
#include "hw/qdev-core.h"
#include "cpu.h"
#include "hw/sysbus.h"
#include "hw/char/serial.h"
#include "hw/misc/unimp.h"
#include "hw/usb/hcd-ehci.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/arm/sprd-sc8810.h"

static void s7568_init(MachineState *machine)
{
    SC8810State *sc8810;
    Error *err = NULL;

    MemoryRegion *sysmem = get_system_memory(); 

    /* Maybe it's possible to load custom internal ROM with this option */
    if (machine->firmware) {
        error_report("BIOS not supported for this machine at present");
        exit(1);
    }

    /* Only allow Cortex-A8 for now before Cortex-A5 support is added */
    if (strcmp(machine->cpu_type, ARM_CPU_TYPE_NAME("cortex-a8")) != 0) {
        error_report("This board only supports cortex-a8 CPU");
        exit(1);
    }

    sc8810 = SPRD_SC8810(object_new(TYPE_SPRD_SC8810));
    object_property_add_child(OBJECT(machine), "soc", OBJECT(sc8810));
    object_unref(OBJECT(sc8810));

    if (!qdev_realize(DEVICE(sc8810), NULL, &err)) {
        error_reportf_err(err, "Couldn't realize Spreadtrum SC8810: ");
        exit(1);
    }

    memory_region_add_subregion(sysmem, memmap[SDRAM_0].base, machine->ram);
}

static void s7568_machine_init(MachineClass *mc)
{
    mc->desc = "Samsung Galaxy Trend GT-S7568";
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("cortex-a8");
    mc->default_ram_size = 1 * GiB; // Actually 768 MiB
    mc->init = s7568_init;
    mc->block_default_type = IF_SD;
    mc->units_per_default_bus = 1;
    mc->ignore_memory_transaction_failures = true;
    mc->default_ram_id = "s7568.ram";
}

DEFINE_MACHINE("samsung-s7568", s7568_machine_init)
