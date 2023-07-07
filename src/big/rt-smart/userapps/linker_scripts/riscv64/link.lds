/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020/12/12     bernard      The first version
 */

OUTPUT_ARCH( "riscv" )

ENTRY(_start)
SECTIONS
{
    /*
     * 64bit userspace: 0x200000000 (default)
     * 32bit userspace: 0xD0000000
     */
    . = 0x200000000;

    .text : 
    {
        __text_start__ = .;
        *(.start);
        *(.text*)                        /* remaining code */

        KEEP(*(.init))
        KEEP(*(.fini))
        
        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)                      /* read-only data (constants) */
        
        KEEP(*(.eh_frame*))

        . = ALIGN(8);
        _etext = .;
        __text_end__ = .;
    }
    
    . = ALIGN(0x1000);  /* must define page align after .text */

        .plt :
    {
        *(*.plt)
    }
    .got :
    {
        __got_start__ = .;
        *(*.got)
        __got_end__ = .;
    }

    data :
    {
        __data_start__ = .;

        *(vtable)
        *(.data*)

        . = ALIGN(8);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(8);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(8);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        . = ALIGN(8);
        /* All data end */
        __data_end__ = .;
    }

    bss :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        *(.stack)
        __bss_end__ = .;
    }
    
    _end = .;
}

