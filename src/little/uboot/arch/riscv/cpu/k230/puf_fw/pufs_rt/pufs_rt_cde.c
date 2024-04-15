
#include "pufs_internal.h"
#include "pufs_rt_internal.h"
#include "platform.h"
// for PSIOT_012CW01D_B12C project

#define PUFS_CDE_SEGMENT          128 // 1K bits
#define PIF_CDE_RWLCK_START_INDEX   4
#define PIF_CDE_RWLCK_MAX_GROUP    24

struct pufs_rt_cde_regs *rt_cde_regs = (struct pufs_rt_cde_regs *)(PUFIOT_ADDR_START+CDE_ADDR_OFFSET);
static int rt_cde_select_index(uint32_t idx)
{
    uint32_t group = idx / RWLOCK_GROUP_OTP;

    if (group >= PIF_CDE_RWLCK_MAX_GROUP)
        return -1;
    
    return PIF_CDE_RWLCK_START_INDEX + group; 
}

/**
 * pufs_read_otp()
 */
pufs_status_t pufs_read_cde(uint8_t* outbuf, uint32_t len, uint32_t addr)
{
    for (uint32_t index = 0; index < len; index+=4)
    {
        *(uint32_t* )(outbuf + index) = rt_cde_regs->otp[(addr+index)/4];
    }

    return SUCCESS;
}

/**
 * pufs_program_otp()
 */
pufs_status_t pufs_program_cde(const uint8_t* inbuf, uint32_t len, uint32_t addr)
{
    for (uint32_t index = 0; index < len; index+=4)
    {
        if(*(uint32_t* )(inbuf + index) == 0) continue;
        if(rt_cde_regs->otp[(addr+index)/4] == *(uint32_t* )(inbuf + index)) continue;
        
        // printf("cde[%d]:0x%x \n", (addr+index)/4, *(uint32_t* )(inbuf + index));
        rt_cde_regs->otp[(addr+index)/4] = *(uint32_t* )(inbuf + index);
    }

    return SUCCESS;
}

pufs_status_t rt_cde_write_lock(uint32_t offset, uint32_t length, pufs_otp_lock_t lock)
{
    uint32_t lock_val = 0, end = 0, start = 0, val32 = 0, rwlock_index, shift, mask = 0;

    switch (lock)
    {
    case RO:
        lock_val = PUFRT_VALUE4(0xC);
        break;
    case RW:
        lock_val = PUFRT_VALUE4(0xF);
        break;
    default:
        return E_INVALID;
    }

    end = (length + (PUFS_CDE_SEGMENT - 1)) / PUFS_CDE_SEGMENT;
    start = offset / PUFS_CDE_SEGMENT;

    for (uint32_t i = 0; i < end; i++)
    {
        int idx = start + i;
        rwlock_index = rt_cde_select_index(idx);

        shift = (idx % RWLOCK_GROUP_OTP) * 4;
        val32 |= lock_val << shift;
        mask |= 0xF << shift;

        if (shift == 28 || i == end - 1)
        {
            val32 |= (rt_regs->pif[rwlock_index] & (~mask));
            rt_regs->pif[rwlock_index] = val32;

            val32 = 0;
            mask = 0;
        }
    }
    return SUCCESS;
}

pufs_otp_lock_t rt_cde_read_lock(uint32_t offset)
{
    int idx;
    uint32_t group_index, lck;

    idx = offset / PUFS_CDE_SEGMENT;
    group_index = idx % RWLOCK_GROUP_OTP;

    if ((idx = rt_cde_select_index(idx)) == -1)
        return N_OTP_LOCK_T;

    lck = (rt_regs->pif[idx] >> (group_index * 4)) & 0xF;

    switch (lck)
    {
    case PUFRT_VALUE4(0x0):
        return RO;
    case PUFRT_VALUE4(0xC):
        return RO;
    case PUFRT_VALUE4(0xF):
        return RW;
    default:
        return N_OTP_LOCK_T;
    }
    return N_OTP_LOCK_T;
}

// mask 1K bits(128 bytes) segment starting from offset input
pufs_status_t rt_cde_write_mask(uint32_t offset)
{
    uint32_t index, group, reg, group_index;
    index = offset / PUFS_CDE_SEGMENT;
    group = index / 16; // 2 bit for each code segment mask of 32 bit register
    group_index = index % 16;

    if (group > 1)
        return E_INVALID;

    reg = rt_regs->ptm[group];
    reg |= 0x3 << (group_index * 2);
    rt_regs->ptm[group] = reg;

    return SUCCESS;
}
