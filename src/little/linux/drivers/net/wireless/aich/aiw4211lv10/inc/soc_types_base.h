/*
 * Copyright (c) 2020-2022 Chengdu Aich Technology Co.,Ltd.All rights reserved.
 * Description: 数据类型定义和公用宏和结构定义
 * Author: CompanyName
 * Create: 2021-06-02
 */

#ifndef TD_TYPES_BASE_H
#define TD_TYPES_BASE_H

#if defined(__CC_ARM)
#define EXT_HAVE_CROSS_COMPILER_ARM_ARMCC
#elif defined(__GNUC__)
#define EXT_HAVE_CROSS_COMPILER_ARM_GCC
#elif defined(__DCC__)
#define EXT_HAVE_CROSS_COMPILER_DIAB
#if defined(__EXT_ASM_LANGUAGE__) || defined(_ASMLANGUAGE)
#define EXT_HAVE_CROSS_COMPILER_DIAB_AS
#endif
#elif defined(_MSC_VER)
#define EXT_HAVE_NOTIVE_COMPILER_VC
#endif

#if !defined(EXT_HAVE_CROSS_COMPILER_DIAB_AS)
#if defined(EXT_HAVE_CROSS_COMPILER_ARM_GCC) || defined(EXT_HAVE_CROSS_COMPILER_ARM_ARMCC) || \
    defined(EXT_HAVE_CROSS_COMPILER_DIAB)
#undef EXT_HAVE_CROSS_COMPILER
#define EXT_HAVE_CROSS_COMPILER
#endif

#ifdef PRODUCT_CFG_OS_WIN
# undef SAL_HAVE_OS_WIN_VER
# undef SAL_HAVE_OS_NU_VER
# undef SAL_HAVE_OS_VX_VER
#define SAL_HAVE_OS_WIN_VER
#endif

#if defined (PRODUCT_CFG_OS_LOS)
# undef SAL_HAVE_OS_VX_VER
# define SAL_HAVE_OS_VX_VER
#else
# if defined (_WIN32) && defined (_MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#  ifndef SAL_HAVE_SIMU_WIN_VER
#   define SAL_HAVE_SIMU_WIN_VER
#   ifdef _USRDLL
#    if !defined(PRODUCT_CFG_MSVC_HIDE_AUTOLINK_OUTPUT_INFO)
#     pragma message("Windows VC Simulator DLL Version ")
#    endif
#   endif
#   ifdef _LIB
#    if !defined(PRODUCT_CFG_MSVC_HIDE_AUTOLINK_OUTPUT_INFO)
#     pragma message("Windows VC Simulator lib Version ")
#    endif
#   endif
#   ifdef _CONSOLE
#    if !defined(PRODUCT_CFG_MSVC_HIDE_AUTOLINK_OUTPUT_INFO)
#     pragma message("Windows VC Simulator console Version ")
#    endif
#   endif
#  else
#   if !defined(PRODUCT_CFG_MSVC_HIDE_AUTOLINK_OUTPUT_INFO)
#    pragma message("Windows Version")
#   endif
#  endif
# endif
#endif

#undef SOC_SYS_DEBUG
#if (defined(PRODUCT_CFG_VERSION_DEBUG) || defined(SAL_HAVE_DEBUG_VERSION)) && !defined(SAL_HAVE_RELEASE_VERSION)
# define SOC_SYS_DEBUG
#endif

#if defined(PRODUCT_CFG_OS_WIN)
#pragma warning(disable:4200)  /* disable nonstandard extension used : zero-sized array in struct/union. */
#pragma warning(disable:4214)  /* allows bitfield structure members to be of any integral type. */
#pragma warning(disable:4201)
#pragma warning(disable:4514)
#pragma warning(disable:4127)
#endif

/* 基本数据类型定义 */
typedef unsigned int            td_u32;
typedef int                     td_s32;
typedef unsigned short          td_u16;
typedef signed   short          td_s16;
typedef unsigned char           td_u8;
typedef signed char             td_s8;
typedef void                    td_void;
typedef char                    td_char;
typedef unsigned char           td_uchar;
typedef td_u8                   td_bool;
typedef void*                   td_pvoid;
typedef td_u8                   td_byte;
typedef td_pvoid                td_handle;
typedef td_byte*                td_pbyte;
typedef float                   td_float;
typedef double                  td_double;
typedef volatile td_u32         td_u32_reg;
typedef td_pvoid                td_func_ptr;
typedef td_u32                  td_func;
typedef unsigned int            td_size_t;
typedef int                     td_ssize_t;
typedef int                     td_offset_t;

/* for 64bits platform, intptr_t/uintptr_t should be defined as 64bits length. */
typedef int                     intptr_t;

#if defined(__LITEOS__)
typedef unsigned int            uintptr_t;
#endif

#undef ERROR
#define ERROR (-1)

/* defines */
#undef NULL
#ifndef NULL
#define NULL 0
#endif

#define TD_CONST               const
#define TD_REG                 register

#define TD_U32_MAX            0xFFFFFFFF
#define TD_U64_MAX            0xFFFFFFFFFFFFFFFFUL
#define TD_U16_MAX            0xFFFF

typedef td_pvoid (*aich_pvoid_callback_f)(td_void);
typedef td_void  (*aich_void_callback_f)(td_void);
typedef td_void  (*aich_void_callback)(td_void);
typedef td_bool  (*aich_bool_callback_f)(td_void);
typedef td_void  (*aich_void_u32_callback_f)(td_u32);
typedef td_u32   (*aich_u32_pvoid_callback_f)(td_pvoid);
typedef td_u32   (*aich_u32_void_callback)(td_void); /* 周边代码待刷新 */
typedef td_u32   (*aich_u32_u32_pvoid_callback_f)(td_pvoid, td_u32);
typedef td_s32   (*funcptr)(td_void);     /* ptr to function returning int */
typedef td_void  (*voidfuncptr)(td_void); /* ptr to function returning void */

typedef TD_CONST td_char*  td_pcstr;

#ifdef PRODUCT_CFG_OS_WIN
typedef unsigned __int64       td_u64;
typedef __int64 td_s64;
#elif defined(EXT_HAVE_CROSS_COMPILER_ARM_ARMCC)
typedef unsigned __int64       td_u64;
typedef __int64 td_s64;
#elif defined(EXT_HAVE_CROSS_COMPILER_ARM_GCC) || defined(HAVE_PCLINT_CHECK)
typedef unsigned long long     td_u64;
typedef long long              td_s64;
#elif defined(EXT_HAVE_CROSS_COMPILER_DIAB)
typedef unsigned long long     td_u64;
typedef long long              td_s64;
#elif !defined(PRODUCT_CFG_HSO)
/* #error "unknown compiler" */
typedef unsigned __int64       td_u64;
typedef __int64 td_s64;
#endif

#define TD_S32_BITS       32
#define TD_S32_MAX       (~(~0 << (TD_S32_BITS - 1)))

#define TD_PUBLIC    extern
#if !defined(PRODUCT_CFG_FEATURE_UT)
# define TD_PRV   static
#else
# define TD_PRV
#endif

# define STATIC   static

#endif /* EXT_HAVE_CROSS_COMPILER_DIAB_AS */

#ifdef PRODUCT_CFG_OS_WIN
# define TD_API   _declspec(dllexport)
# define TD_INLINE  __inline
#elif defined(EXT_HAVE_CROSS_COMPILER_ARM_GCC)
#  define TD_INLINE  inline
#  define TD_API
#elif defined(EXT_HAVE_CROSS_COMPILER_ARM_ARMCC)
#  define TD_INLINE  inline
#  define TD_API
#elif defined(EXT_HAVE_CROSS_COMPILER_DIAB)
#  define TD_INLINE  __inline__
#  define TD_API
# else
#  define TD_INLINE __inline
#  define TD_API
#endif

#define TD_PRVL TD_PRV TD_INLINE

#if defined(__ONEBUILDER__CROSS_COMPILER_PRODUCT_CONFIG__)
#if defined(EXT_HAVE_CROSS_COMPILER_ARM_ARMCC) || defined(EXT_HAVE_CROSS_COMPILER_ARM_GCC)
# define td_section(name_string) __attribute__ ((section(name_string)))
# define TD_PACKED               __attribute__((packed))
# define TD_ALIGNED4             __attribute__ ((aligned (4)))
#elif defined(EXT_HAVE_CROSS_COMPILER_DIAB)
# define td_section(name_string) __attribute__ ((section(name_string)))
# define TD_PACKED               __attribute__((packed))
# define TD_ALIGNED4             __attribute__ ((aligned (4)))
#endif
#elif defined(SAL_HAVE_OS_WIN_VER) || defined(PRODUCT_CFG_HSO)
#  define td_section(name_string)
#  define TD_PACKED
#  define TD_ALIGNED4
#else
#  define td_section(name_string)
#  define TD_PACKED
#  define TD_ALIGNED4
#endif

#if defined(SAL_HAVE_OS_WIN_VER)
# if defined(_DEBUG) || defined(PRODUCT_CFG_VERSION_DEBUG)
#  define aich_dll_lib_name(x)        x ## "_debug.dll"
# else
#  define aich_dll_lib_name(x)        x ## "_release.dll"
# endif
#else
#if defined(EXT_HAVE_CROSS_COMPILER_ARM_GCC)
# define aich_dll_lib_name(x)        x
#else
# define aich_dll_lib_name(x)        x ## ".lib"
#endif
#endif

#if defined(SAL_HAVE_NO_EXTERN_DEFINED)
# define TD_EXTERN
# define TD_EXTERN_C
#else
# if defined(PRODUCT_CFG_OS_WIN)
#  define TD_EXTERN      extern TD_API
#  define TD_EXTERN_C    TD_EXTERN
#  define TD_EAPI        extern TD_API
# else
#    define TD_EXTERN   extern
#    define TD_EAPI
#    define TD_EXTERN_C
# endif
#endif

#ifdef __cplusplus
# define EXT_CPP_START    extern "C" {
# define EXT_CPP_END      }
#else
# define EXT_CPP_START
# define EXT_CPP_END
#endif

#if defined(EXT_HAVE_CROSS_COMPILER_ARM_ARMCC)
#define TD_NOP             __asm { nop }
#define aich_dbg_break()     __asm { swi 0x14DEAD }
#elif defined(EXT_HAVE_CROSS_COMPILER_DIAB)
#define TD_NOP
#define aich_dbg_break()
#else
#define TD_NOP
#ifdef PRODUCT_CFG_OS_WIN
#define aich_dbg_break()     _asm { int 3 }
#else
#define aich_dbg_break()
#endif
#endif

#define EXT_START_HEADER    EXT_CPP_START
#define EXT_END_HEADER      EXT_CPP_END

#undef TD_OUT
#undef TD_IN
#undef TD_INOUT
#define TD_OUT
#define TD_IN
#define TD_INOUT

#define TD_FALSE         0
#define TD_TRUE          1
#define TD_SWITCH_OFF    0
#define TD_SWITCH_ON     1

#ifdef __cplusplus
#define TD_NULL       0
#else
#define TD_NULL    ((void *)0)
#endif


#define aich_array_count(x)    (sizeof(x) / sizeof((x)[0]))

#if !defined(aich_unref_param) && !defined(EXT_HAVE_CROSS_COMPILER_DIAB)
#define aich_unref_param(P)  ((P) = (P))
#else
#define aich_unref_param(P)
#endif

#define TD_VOLATILE
#define aich_sys_get_lr()                        0

#define aich_aligin_u32_size(x)  (((x) & (~3)) + 4) /* 构造4个字节对齐 */
#define aich_is_align_u32(x)     (!((x) & 3))       /* 判断是否为4字节对齐 */
#define aich_is_unalign_u32(x)   ((x) & 3)          /* 判断是否为4字节对齐 */
#if defined(HAVE_PCLINT_CHECK)
#define aich_fieldoffset(s, m)    (0)
#else
#define aich_fieldoffset(s, m)    ((td_u32)&(((s *)0)->m)) /* 结构成员偏移 */
#endif

#define TD_CHAR_CR             '\r' /* 0x0D */
#define TD_CHAR_LF             '\n' /* 0x0A */
#define aich_tolower(x)          ((x) | 0x20)  /* Works only for digits and letters, but small and fast */

#define aich_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))
#define aich_makeu16(a, b)       ((td_u16)(((td_u8)(a))  | ((td_u16)((td_u8)(b))) << 8))
#define aich_makeu32(a, b)       ((td_u32)(((td_u16)(a)) | ((td_u32)((td_u16)(b))) << 16))
#define aich_makeu64(a, b)       ((td_u64)(((td_u32)(a)) | ((td_u64)((td_u32)(b))) <<32))
#define aich_joinu32(a, b, c, d) ((a) | ((td_u32)(b) << 8) | ((td_u32)(c) << 16) | ((td_u32)(d) << 24))

#define aich_hiu32(l)            ((td_u32)(((td_u64)(l) >> 32) & 0xFFFFFFFF))
#define aich_lou32(l)            ((td_u32)(l))

#define aich_hiu16(l)            ((td_u16)(((td_u32)(l) >> 16) & 0xFFFF))
#define aich_lou16(l)            ((td_u16)(l))
#define aich_hiu8(l)             ((td_u8)(((td_u16)(l) >> 8) & 0xFF))
#define aich_lou8(l)             ((td_u8)(l))

#define aich_max(a, b)            (((a) > (b)) ? (a) : (b))
#define aich_min(a, b)            (((a) < (b)) ? (a) : (b))
#define aich_sub(a, b)            (((a) > (b)) ? ((a) - (b)) : 0)
#define aich_abs_sub(a, b)       (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define aich_byte_align(value, align)            (((value) + (align) - 1) & (~((align) -1)))
#define aich_is_byte_align(value, align)         (((td_u32)(value) & ((align) - 1))== 0)
#define aich_inc_wraparound(value, round_size)   ++(value); (value) &= ((round_size) - 1)
#define aich_dec_wraparound(value, round_size)   --(value); (value) &= ((round_size) - 1)

#define aich_swap_byteorder_16(value)            ((((value) & 0xFF) << 8) + (((value) & 0xFF00) >> 8))

#define aich_swap_byteorder_32(value)       \
    ((td_u32)(((value) & 0x000000FF) << 24) + \
     (td_u32)(((value) & 0x0000FF00)  << 8) +  \
     (td_u32)(((value) & 0x00FF0000)  >> 8) +  \
     (td_u32)(((value) & 0xFF000000)  >> 24))

#define aich_swap_byteorder_64(value)                     \
    ((((value) & 0x00000000000000ffULL) << 56) +    \
     (((value) & 0x000000000000ff00ULL) << 40) +     \
     (((value) & 0x0000000000ff0000ULL) << 24) +     \
     (((value) & 0x00000000ff000000ULL) << 8) +      \
     (((value) & 0x000000ff00000000ULL) >> 8) +      \
     (((value) & 0x0000ff0000000000ULL) >> 24) +     \
     (((value) & 0x00ff000000000000ULL) >> 40) +     \
     (((value) & 0xff00000000000000ULL) >> 56))

#undef MIN_T
#define MIN_T  aich_min

#define aich_make_identifier(a, b, c, d)      aich_makeu32(aich_makeu16(a, b), aich_makeu16(c, d))
#define aich_make_ver16(spc, b)           ((td_u16)(((td_u8)((spc)&0x0F))  | ((td_u16)((td_u8)(b))) << 12))

#define  aich_set_bit_i(val, n)               ((val) |= (1 << (n)))
#define  aich_clr_bit_i(val, n)               ((val) &= ~(1 << (n)))
#define  aich_is_bit_set_i(val, n)            ((val) & (1 << (n)))
#define  aich_is_bit_clr_i(val, n)            (~((val) & (1 << (n))))
#define  aich_switch_bit_i(val, n)            ((val) ^= (1 << (n)))
#define  aich_get_bit_i(val, n)               (((val) >> (n)) & 1)
#define  aich_reg_clr_bit_i(reg, n)           ((*(volatile unsigned int *)(reg)) &= ~(1 << (n)))

#define  aich_u8_bit_val(b7, b6, b5, b4, b3, b2, b1, b0)                       \
    (((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))
#define  aich_u16_bit_val(b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0)    \
    (td_u16)(((b12) << 12) | ((b11) << 11) | ((b10) << 10) | ((b9) << 9) | ((b8) << 8) | ((b7) << 7) |   \
    ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))

#if defined(__ONEBUILDER__CROSS_COMPILER_PRODUCT_CONFIG__)
#define TD_ENUM    TD_ALIGNED4 enum
#define TD_U8A      TD_ALIGNED4 td_u8
#define TD_U16A     TD_ALIGNED4 td_u16
#define EXT_CHARTAA   TD_ALIGNED4 td_char
#else
#define TD_ENUM    enum
#define TD_U8A      td_u8
#define TD_U16A     td_u16
#define EXT_CHARTAA   td_char
#endif

#if defined(PRODUCT_CFG_HSO) && defined(EXT_HAVE_NOTIVE_COMPILER_VC)
#define __FUNCTION__   "NA"
#endif

/*****************************************************************************/
#define aich_set_u32_ptr_val(ptr, offset, val)  (*((td_u32*)(((td_u8*)(ptr)) + (offset))) = (val))
#define aich_get_u32_ptr_val(ptr, offset)      *((td_u32*)(((td_u8*)(ptr)) + (offset)))
/*****************************************************************************/
/*****************************************************************************/
#define EXT_SIZE_1K                     1024
#define EXT_SIZE_1M                     (1024 * 1024)
/*****************************************************************************/
#ifndef bit
#define bit(x)                         (1UL << (x))
#endif
#define BIT31                          ((td_u32)(1UL << 31))
#define BIT30                          ((td_u32)(1 << 30))
#define BIT29                          ((td_u32)(1 << 29))
#define BIT28                          ((td_u32)(1 << 28))
#define BIT27                          ((td_u32)(1 << 27))
#define BIT26                          ((td_u32)(1 << 26))
#define BIT25                          ((td_u32)(1 << 25))
#define BIT24                          ((td_u32)(1 << 24))
#define BIT23                          ((td_u32)(1 << 23))
#define BIT22                          ((td_u32)(1 << 22))
#define BIT21                          ((td_u32)(1 << 21))
#define BIT20                          ((td_u32)(1 << 20))
#define BIT19                          ((td_u32)(1 << 19))
#define BIT18                          ((td_u32)(1 << 18))
#define BIT17                          ((td_u32)(1 << 17))
#define BIT16                          ((td_u32)(1 << 16))
#define BIT15                          ((td_u32)(1 << 15))
#define BIT14                          ((td_u32)(1 << 14))
#define BIT13                          ((td_u32)(1 << 13))
#define BIT12                          ((td_u32)(1 << 12))
#define BIT11                          ((td_u32)(1 << 11))
#define BIT10                          ((td_u32)(1 << 10))
#define BIT9                           ((td_u32)(1 << 9))
#define BIT8                           ((td_u32)(1 << 8))
#define BIT7                           ((td_u32)(1 << 7))
#define BIT6                           ((td_u32)(1 << 6))
#define BIT5                           ((td_u32)(1 << 5))
#define BIT4                           ((td_u32)(1 << 4))
#define BIT3                           ((td_u32)(1 << 3))
#define BIT2                           ((td_u32)(1 << 2))
#define BIT1                           ((td_u32)(1 << 1))
#define BIT0                           ((td_u32)(1 << 0))

#define HALFWORD_BIT_WIDTH              16

/* 寄存器访问接口 */
#define aich_reg_write(addr, val)        (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define aich_reg_read(addr, val)         ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define aich_reg_write32(addr, val)      (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define aich_reg_read32(addr, val)       ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define aich_reg_read_val32(addr)        (*(volatile unsigned int*)(uintptr_t)(addr))
#define aich_reg_setbitmsk(addr, msk)    ((aich_reg_read_val32(addr)) |= (msk))
#define aich_reg_clrbitmsk(addr, msk)    ((aich_reg_read_val32(addr)) &= ~(msk))
#define aich_reg_clrbit(addr, pos)       ((aich_reg_read_val32(addr)) &= ~((unsigned int)(1) << (pos)))
#define aich_reg_setbit(addr, pos)       ((aich_reg_read_val32(addr)) |= ((unsigned int)(1) << (pos)))
#define aich_reg_clrbits(addr, pos, bits) (aich_reg_read_val32(addr) &= ~((((unsigned int)1 << (bits)) - 1) << (pos)))
#define aich_reg_setbits(addr, pos, bits, val) (aich_reg_read_val32(addr) =           \
    (aich_reg_read_val32(addr) & (~((((unsigned int)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned int)((val) & (((unsigned int)1 << (bits)) - 1)) << (pos)))
#define aich_reg_getbits(addr, pos, bits) ((aich_reg_read_val32(addr) >> (pos)) & (((unsigned int)1 << (bits)) - 1))

#define aich_reg_write16(addr, val)      (*(volatile unsigned short *)(uintptr_t)(addr) = (val))
#define aich_reg_read16(addr, val)       ((val) = *(volatile unsigned short *)(uintptr_t)(addr))
#define aich_reg_read_val16(addr)        (*(volatile unsigned short*)(uintptr_t)(addr))
#define aich_reg_clrbit16(addr, pos)       ((aich_reg_read_val16(addr)) &= ~((unsigned short)(1) << (pos)))
#define aich_reg_setbit16(addr, pos)       ((aich_reg_read_val16(addr)) |= ((unsigned short)(1) << (pos)))
#define aich_reg_clrbits16(addr, pos, bits) (aich_reg_read_val16(addr) &= ~((((unsigned short)1 << (bits)) - 1) << (pos)))
#define aich_reg_setbits16(addr, pos, bits, val) (aich_reg_read_val16(addr) =           \
    (aich_reg_read_val16(addr) & (~((((unsigned short)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned short)((val) & (((unsigned short)1 << (bits)) - 1)) << (pos)))
#define aich_reg_getbits16(addr, pos, bits) ((aich_reg_read_val16(addr) >> (pos)) & (((unsigned short)1 << (bits)) - 1))

#define reg_write32(addr, val) (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define reg_read32(addr, val)  ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define reg_read_val(addr)   (*(volatile unsigned *)(uintptr_t)(addr))

#ifndef BSP_RAM_TEXT_SECTION
#define BSP_RAM_TEXT_SECTION       __attribute__ ((section(".bsp.ram.text")))
#endif

#ifndef BSP_ROM_RODATA_SECTION
#define BSP_ROM_RODATA_SECTION      __attribute__ ((section(".bsp.rom.rodata")))
#endif

#ifndef BSP_ROM_DATA0_SECTION
#define BSP_ROM_DATA0_SECTION       __attribute__ ((section(".bsp.rom.data0")))
#endif

#ifndef ROM_TEXT_PATCH_SECTION
#define ROM_TEXT_PATCH_SECTION       __attribute__ ((section(".rom.text.patch")))
#endif

#ifndef LP_RAM_BSS_SECTION
#define LP_RAM_BSS_SECTION           __attribute__ ((section(".lowpower.ram.bss")))
#endif

#ifndef ROM_DATA_PATCH_SECTION
#define ROM_DATA_PATCH_SECTION       __attribute__ ((section(".rom.data.patch")))
#endif

#ifdef HAVE_PCLINT_CHECK
#define aich_likely(x)    (x)
#define aich_unlikely(x)  (x)
#else
#define aich_likely(x) __builtin_expect(!!(x), 1)
#define aich_unlikely(x) __builtin_expect(!!(x), 0)
#endif
#define TD_ALWAYS_STAIC_INLINE __attribute__((always_inline)) static inline

#ifdef HAVE_PCLINT_CHECK
#define aich_offset_of_member(type, member)   0
#else
#define aich_offset_of_member(type, member)   ((td_u32)(&((type *)0)->member))
#endif
#define BITS_PER_BYTE   8
#define HEXADECIMAL     16
#define DECIMAL         10
#define SZ_1KB 1024
#define SZ_1MB (SZ_1KB * SZ_1KB)
#define SZ_4KB 4096
/*****************************************************************************/
#include <soc_errno.h>

#define EXT_SYS_WAIT_FOREVER           0xFFFFFFFF

#endif /* __SOC_TYPES__BASE_H__ */

