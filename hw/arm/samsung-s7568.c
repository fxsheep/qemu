#include "qemu/osdep.h"
#include "exec/address-spaces.h"
#include "qapi/error.h"
#include "qemu/error-report.h"
#include "qemu/module.h"
#include "qemu/units.h"
#include "qemu/datadir.h"
#include "hw/qdev-core.h"
#include "cpu.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/char/serial.h"
#include "hw/misc/unimp.h"
#include "hw/usb/hcd-ehci.h"
#include "sysemu/sysemu.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/arm/sprd-sc8810.h"

static struct arm_boot_info s7568_binfo = {
    .loader_start = SDRAM_0,
    .board_id = 0x7DD,
};

static void s7568_init(MachineState *machine)
{
    SC8810State *sc8810;
    Error *err = NULL;

    MemoryRegion *sysmem = get_system_memory(); 
    MemoryRegion *irom = g_new(MemoryRegion, 1);
    int irom_size;
    char *filename;

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

    memory_region_init_rom(irom, NULL, "sc8810.irom", memmap[IROM_0].size,
                           &error_fatal);
    memory_region_add_subregion(sysmem, memmap[IROM_0].base, irom);
    filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, machine->firmware);
    if (filename) {
        irom_size = load_image_targphys(filename, memmap[IROM_0].base,
                                        memmap[IROM_0].size);
        g_free(filename);
    } else {
        irom_size = -1;
    }
    if (machine->firmware) {
        if (irom_size < 0 || irom_size > memmap[IROM_0].size) {
            error_report("Could not load sc8810 irom '%s'", machine->firmware);
            exit(1);
        } else {
            CPUState *cs = CPU(&sc8810->cpu);    
            cpu_reset(cs);
            cpu_set_pc(cs, memmap[IROM_0].base); 
        }
    }
    else {
        s7568_binfo.ram_size = machine->ram_size;
        arm_load_kernel(&sc8810->cpu, machine, &s7568_binfo);
    }
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
