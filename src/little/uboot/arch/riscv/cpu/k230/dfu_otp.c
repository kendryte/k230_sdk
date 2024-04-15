#include <common.h>
#include <dfu.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include "pufs_rt.h"

#define ALT_OTP         0
#define ALT_OTP_LOCK    1
#define ALT_CDE         2
#define ALT_CDE_LOCK    3

int dfu_write_medium_virt(struct dfu_entity *dfu, u64 offset,
				 void *buf, long *len)
{
    printf("%s: dev_num=%d off=0x%llx, len=0x%x\n", __func__, dfu->data.virt.dev_num, offset, (u32)*len);
    if(offset !=0)
    {
        return 0;
    }
    if(dfu->data.virt.dev_num == ALT_OTP)
    {
        if(*len == 1024)
        {
            pufs_program_otp(buf, 1024, 0);
        }
    }
    else if(dfu->data.virt.dev_num == ALT_OTP_LOCK)
    {
        if(*len == 1024)
        {
            uint32_t *otp_rwlck = buf;
            for(int i=0; i<1024/4; i++)
            {
                if(otp_rwlck[i] == 0x12345678)
                {
                    pufs_lock_otp(i*4, 4, RO);
                }
                else if(otp_rwlck[i] == 0x9abcdef0)
                {
                    pufs_lock_otp(i*4, 4, NA);
                }
            }
        }
    }
    else if(dfu->data.virt.dev_num == ALT_CDE)
    {
        if(*len == 3072)
        {
            pufs_program_cde(buf, 3072, 0);
        }
    }
    else if(dfu->data.virt.dev_num == ALT_CDE_LOCK)
    {
        if(*len == 96)
        {
            uint32_t *cde_rolck = buf;
            for(int i=0; i<96/4; i++)
            {
                if(cde_rolck[i] == 0x12345678)
                {
                    rt_cde_write_lock(i*128, 128, RO);
                }
            }
        }
    }
    else
    {
        printf("%s: dev_num=%d\n", __func__, dfu->data.virt.dev_num);
    }
	
	return 0;
}

int dfu_get_medium_size_virt(struct dfu_entity *dfu, u64 *size)
{
    printf("%s: dev_num=%d\n", __func__, dfu->data.virt.dev_num);
    if(dfu->data.virt.dev_num == ALT_OTP)
    {
        *size = 1024;
    }
    else if(dfu->data.virt.dev_num == ALT_OTP_LOCK)
    {
        *size = 1024;//1024B 4B为一个保护单位，每个保护单位使用4B；
    }
    else if(dfu->data.virt.dev_num == ALT_CDE)
    {
        *size = 3072;
    }
    else if(dfu->data.virt.dev_num == ALT_CDE_LOCK)
    {
        *size = 3072/128*4; //3072B 128B为一个保护单位，每个保护单位使用4B；96
    }
    else
    {
        printf("%s: dev_num=%d\n", __func__, dfu->data.virt.dev_num);
    }

	return 0;
}

int dfu_read_medium_virt(struct dfu_entity *dfu, u64 offset,
				void *buf, long *len)
{
    printf("%s: dev_num=%d off=0x%llx, len=0x%x\n", __func__, dfu->data.virt.dev_num, offset, (u32)*len);
    if(offset !=0)
    {
        return 0;
    }
    if(dfu->data.virt.dev_num == ALT_OTP)
    {
        pufs_read_otp(buf, 1024, 0);
        *len = 1024;
    }
    else if(dfu->data.virt.dev_num == ALT_OTP_LOCK)
    {
        uint32_t *otp_rwlck = buf;
        for(int i=0; i<1024/4; i++)
        {
            pufs_otp_lock_t rwclk = pufs_get_otp_rwlck(i*4);
            if( rwclk == RO)
            {
                otp_rwlck[i] = 0x12345678;
            }
            else if( rwclk == NA)
            {
                otp_rwlck[i] = 0x9abcdef0;
            }
            else {
                otp_rwlck[i] = 0x0;
            }

        }
        
        *len = 1024;
    }
    else if(dfu->data.virt.dev_num == ALT_CDE)
    {
        pufs_read_cde(buf, 3072, 0);
        *len = 3072;
    }
    else if(dfu->data.virt.dev_num == ALT_CDE_LOCK)
    {
        uint32_t *cde_rolck = buf;
        for(int i=0; i<3072/128; i++)
        {
            if(rt_cde_read_lock(i*128) == RO)
            {
                cde_rolck[i] = 0x12345678;
            }
            else {
                cde_rolck[i] = 0x0;
            }

        }

        *len = 3072/128*4;
    }
    else
    {
        printf("%s: dev_num=%d\n", __func__, dfu->data.virt.dev_num);
        *len = 0;
    }

	

	return 0;
}