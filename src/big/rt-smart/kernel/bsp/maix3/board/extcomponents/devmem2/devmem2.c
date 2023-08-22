#include <rtthread.h>
#include <rthw.h>
#include <ctype.h>
#include <stdlib.h>
#include <ioremap.h>
#include "riscv_mmu.h"

#define MAP_SIZE    PAGE_SIZE
#define MAP_MASK    (MAP_SIZE - 1)

int devmem2(int argc, char **argv) {
    int fd;
    volatile void *virt_addr = RT_NULL;
    void *map_base = RT_NULL;
    volatile rt_uint64_t read_result, writeval;
    rt_ubase_t target;
    int access_type = 'w';

    if(argc < 2) {
        rt_kprintf("\nUsage:\t%s { address } [ type [ data ] ]\n"
            "\taddress : memory address to act upon\n"
            "\ttype    : access operation type : [b]yte, [h]alfword, [w]ord [d]word\n"
            "\tdata    : data to be written\n\n",
            argv[0]);
        return -1;
    }
    target = strtoul(argv[1], 0, 0);
    if(target & 0x3) {
        rt_kprintf("The address must be 8-byte aligned!\n");
        return -1;
    }

    if(argc > 2)
        access_type = tolower(argv[2][0]);

    map_base = rt_ioremap_nocache((void *)(target & ~MAP_MASK), MAP_SIZE);
    virt_addr = map_base + (target & MAP_MASK);

    switch(access_type) {
        case 'b':
            read_result = *((rt_uint8_t *) virt_addr);
            break;
        case 'h':
            read_result = *((rt_uint16_t *) virt_addr);
            break;
        case 'w':
            read_result = *((rt_uint32_t *) virt_addr);
            break;
        case 'd':
            read_result = *((rt_uint64_t *) virt_addr);
            break;
        default:
            rt_iounmap(map_base);
            rt_kprintf("Illegal data type '%c'.\n", access_type);
            return -1;
    }
    rt_kprintf("Value at address 0x%lX (%p): 0x%lX\n", target, virt_addr, read_result);

    if(argc > 3) {
        writeval = strtoul(argv[3], 0, 0);
        switch(access_type) {
            case 'b':
                *((rt_uint8_t *) virt_addr) = writeval;
                read_result = *((rt_uint8_t *) virt_addr);
                break;
            case 'h':
                *((rt_uint16_t *) virt_addr) = writeval;
                read_result = *((rt_uint16_t *) virt_addr);
                break;
            case 'w':
                *((rt_uint32_t *) virt_addr) = writeval;
                read_result = *((rt_uint32_t *) virt_addr);
                break;
            case 'd':
                *((rt_uint64_t *) virt_addr) = writeval;
                read_result = *((rt_uint64_t *) virt_addr);
                break;
        }
        rt_kprintf("Written 0x%X; readback 0x%lX\n", writeval, read_result);
    }
    rt_iounmap(map_base);
    return 0;
}

MSH_CMD_EXPORT(devmem2, Simple program to read/write from/to any location in memory);