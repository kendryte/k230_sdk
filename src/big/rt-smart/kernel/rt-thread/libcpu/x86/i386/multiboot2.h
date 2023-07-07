/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2021-07-17     JasonHu,GuEe-GUI  used in i386
 */

#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

/*  How many bytes from the start of the file we search for the header. */
#define MULTIBOOT_SEARCH                        32768
#define MULTIBOOT_HEADER_ALIGN                  8

/*  The magic field should contain this. */
#define MULTIBOOT2_HEADER_MAGIC                 0xe85250d6

/*  This should be in %eax. */
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289

/*  Alignment of multiboot modules. */
#define MULTIBOOT_MOD_ALIGN                     0x00001000

/*  Alignment of the multiboot info structure. */
#define MULTIBOOT_INFO_ALIGN                    0x00000008

/*  Flags set in the ’flags’ member of the multiboot header. */

#define MULTIBOOT_TAG_ALIGN                  8
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP          17
#define MULTIBOOT_TAG_TYPE_EFI_BS            18
#define MULTIBOOT_TAG_TYPE_EFI32_IH          19
#define MULTIBOOT_TAG_TYPE_EFI64_IH          20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR    21

#define MULTIBOOT_HEADER_TAG_END  0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  1
#define MULTIBOOT_HEADER_TAG_ADDRESS  2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS  4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN  6
#define MULTIBOOT_HEADER_TAG_EFI_BS        7
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32  8
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64  9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE  10

#define MULTIBOOT_ARCHITECTURE_I386  0
#define MULTIBOOT_ARCHITECTURE_MIPS32  4
#define MULTIBOOT_HEADER_TAG_OPTIONAL 1

#define MULTIBOOT_LOAD_PREFERENCE_NONE 0
#define MULTIBOOT_LOAD_PREFERENCE_LOW 1
#define MULTIBOOT_LOAD_PREFERENCE_HIGH 2

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED 1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 2

#ifndef __ASSEMBLY__

struct multiboot_header
{
  /*  Must be MULTIBOOT_MAGIC - see above. */
  rt_uint32_t magic;

  /*  ISA */
  rt_uint32_t architecture;

  /*  Total header length. */
  rt_uint32_t header_length;

  /*  The above fields plus this one must equal 0 mod 2^32. */
  rt_uint32_t checksum;
};

struct multiboot_header_tag
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
};

struct multiboot_header_tag_information_request
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
  rt_uint32_t requests[0];
};

struct multiboot_header_tag_address
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
  rt_uint32_t header_addr;
  rt_uint32_t load_addr;
  rt_uint32_t load_end_addr;
  rt_uint32_t bss_end_addr;
};

struct multiboot_header_tag_entry_address
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
  rt_uint32_t entry_addr;
};

struct multiboot_header_tag_console_flags
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
  rt_uint32_t console_flags;
};

struct multiboot_header_tag_framebuffer
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
  rt_uint32_t width;
  rt_uint32_t height;
  rt_uint32_t depth;
};

struct multiboot_header_tag_module_align
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
};

struct multiboot_header_tag_relocatable
{
  rt_uint16_t type;
  rt_uint16_t flags;
  rt_uint32_t size;
  rt_uint32_t min_addr;
  rt_uint32_t max_addr;
  rt_uint32_t align;
  rt_uint32_t preference;
};

struct multiboot_color
{
  rt_uint8_t red;
  rt_uint8_t green;
  rt_uint8_t blue;
};

struct multiboot_mmap_entry
{
  rt_uint64_t addr;
  rt_uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
  rt_uint32_t type;
  rt_uint32_t zero;
};
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

struct multiboot_tag
{
  rt_uint32_t type;
  rt_uint32_t size;
};

struct multiboot_tag_string
{
  rt_uint32_t type;
  rt_uint32_t size;
  char string[0];
};

struct multiboot_tag_module
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t mod_start;
  rt_uint32_t mod_end;
  char cmdline[0];
};

struct multiboot_tag_basic_meminfo
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t mem_lower;
  rt_uint32_t mem_upper;
};

struct multiboot_tag_bootdev
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t biosdev;
  rt_uint32_t slice;
  rt_uint32_t part;
};

struct multiboot_tag_mmap
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t entry_size;
  rt_uint32_t entry_version;
  struct multiboot_mmap_entry entries[0];
};

struct multiboot_vbe_info_block
{
  rt_uint8_t external_specification[512];
};

struct multiboot_vbe_mode_info_block
{
  rt_uint8_t external_specification[256];
};

struct multiboot_tag_vbe
{
  rt_uint32_t type;
  rt_uint32_t size;

  rt_uint16_t vbe_mode;
  rt_uint16_t vbe_interface_seg;
  rt_uint16_t vbe_interface_off;
  rt_uint16_t vbe_interface_len;

  struct multiboot_vbe_info_block vbe_control_info;
  struct multiboot_vbe_mode_info_block vbe_mode_info;
};

struct multiboot_tag_framebuffer_common
{
  rt_uint32_t type;
  rt_uint32_t size;

  rt_uint64_t framebuffer_addr;
  rt_uint32_t framebuffer_pitch;
  rt_uint32_t framebuffer_width;
  rt_uint32_t framebuffer_height;
  rt_uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
  rt_uint8_t framebuffer_type;
  rt_uint16_t reserved;
};

struct multiboot_tag_framebuffer
{
  struct multiboot_tag_framebuffer_common common;

  union
  {
    struct
    {
      rt_uint16_t framebuffer_palette_num_colors;
      struct multiboot_color framebuffer_palette[0];
    };
    struct
    {
      rt_uint8_t framebuffer_red_field_position;
      rt_uint8_t framebuffer_red_mask_size;
      rt_uint8_t framebuffer_green_field_position;
      rt_uint8_t framebuffer_green_mask_size;
      rt_uint8_t framebuffer_blue_field_position;
      rt_uint8_t framebuffer_blue_mask_size;
    };
  };
};

struct multiboot_tag_elf_sections
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t num;
  rt_uint32_t entsize;
  rt_uint32_t shndx;
  char sections[0];
};

struct multiboot_tag_apm
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint16_t version;
  rt_uint16_t cseg;
  rt_uint32_t offset;
  rt_uint16_t cseg_16;
  rt_uint16_t dseg;
  rt_uint16_t flags;
  rt_uint16_t cseg_len;
  rt_uint16_t cseg_16_len;
  rt_uint16_t dseg_len;
};

struct multiboot_tag_efi32
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t pointer;
};

struct multiboot_tag_efi64
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint64_t pointer;
};

struct multiboot_tag_smbios
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint8_t major;
  rt_uint8_t minor;
  rt_uint8_t reserved[6];
  rt_uint8_t tables[0];
};

struct multiboot_tag_old_acpi
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint8_t rsdp[0];
};

struct multiboot_tag_new_acpi
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint8_t rsdp[0];
};

struct multiboot_tag_network
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint8_t dhcpack[0];
};

struct multiboot_tag_efi_mmap
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t descr_size;
  rt_uint32_t descr_vers;
  rt_uint8_t efi_mmap[0];
};

struct multiboot_tag_efi32_ih
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t pointer;
};

struct multiboot_tag_efi64_ih
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint64_t pointer;
};

struct multiboot_tag_load_base_addr
{
  rt_uint32_t type;
  rt_uint32_t size;
  rt_uint32_t load_base_addr;
};

#endif /* __ASSEMBLY__ */

#endif /* __MULTIBOOT_H__ */
