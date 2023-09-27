/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <asm/asm.h>
#include <asm/io.h>
#include <asm/types.h>
#include <lmb.h>
#include <cpu_func.h>
#include <stdio.h>
#include <common.h>
#include <command.h>
#include <image.h>
#include <gzip.h>
#include <asm/spl.h>
#include "sysctl.h"

#include <pufs_hmac.h>
#include <pufs_ecp.h>
#include <pufs_rt.h>
#include "pufs_sm2.h"
#include <pufs_sp38a.h>
#include <pufs_sp38d.h>
#include <linux/kernel.h>
#include "k230_board_common.h"
#include <mmc.h>
#include <spi_flash.h>
#include <dm/device-internal.h>
#include <spl.h>
#include <spi.h>
#include <nand.h>
#include <linux/mtd/mtd.h>
#include "sdk_autoconf.h"
#include <linux/delay.h>

static int k230_check_and_get_plain_data(firmware_head_s *pfh, ulong *pplain_addr);
static int k230_boot_rtt_uimage(image_header_t *pUh);

#define LINUX_KERNEL_IMG_MAX_SIZE  (25*1024*1024)

#ifndef CONFIG_MEM_LINUX_SYS_SIZE 
#define  CONFIG_MEM_LINUX_SYS_SIZE  CONFIG_MEM_RTT_SYS_SIZE
#endif 

#ifndef CONFIG_MEM_LINUX_SYS_BASE 
#define  CONFIG_MEM_LINUX_SYS_BASE CONFIG_MEM_RTT_SYS_BASE
#endif 



unsigned long get_CONFIG_CIPHER_ADDR(void)
{
    unsigned long ret=0;
    
    if(CONFIG_MEM_LINUX_SYS_SIZE >= 0x8000000){
        ret = round_down( (((CONFIG_MEM_LINUX_SYS_SIZE - 0x1000000)/3) + CONFIG_MEM_LINUX_SYS_BASE ), 0x100000);//25MB
    }else{
        ret = round_down ((LINUX_KERNEL_IMG_MAX_SIZE + CONFIG_MEM_LINUX_SYS_BASE ), 0x100000);//25MB;
    }

    
    return ret;
}
unsigned long get_CONFIG_PLAIN_ADDR(void)
{
    unsigned long ret=0;
    if(CONFIG_MEM_LINUX_SYS_SIZE >= 0x8000000){
        ret = round_down( ((CONFIG_MEM_LINUX_SYS_SIZE - 0x1000000)/3*2) + CONFIG_MEM_LINUX_SYS_BASE , 0x100000);
    }else {
        ret = round_down ((CONFIG_MEM_LINUX_SYS_SIZE- 0x1000000 - LINUX_KERNEL_IMG_MAX_SIZE)/2 + CONFIG_CIPHER_ADDR, 0x100000);//25MB;
    }
    return ret;    
}

#define USE_UBOOT_BOOTARGS 
#define OPENSBI_DTB_ADDR ( CONFIG_MEM_LINUX_SYS_BASE +0x2000000)
#define RAMDISK_ADDR ( CONFIG_MEM_LINUX_SYS_BASE +0x2000000 + 0X100000)

#define SUPPORT_MMC_LOAD_BOOT

sysctl_boot_mode_e g_bootmod = SYSCTL_BOOT_MAX;

#ifdef USE_UBOOT_BOOTARGS
//weak function
char *board_fdt_chosen_bootargs(void){
    char *bootargs = env_get("bootargs");
    if(NULL == bootargs) {
        if(g_bootmod == SYSCTL_BOOT_SDIO0)
            bootargs = "root=/dev/mmcblk0p3 loglevel=8 rw rootdelay=4 rootfstype=ext4 console=ttyS0,115200 crashkernel=256M-:128M earlycon=sbi";
        else if(g_bootmod == SYSCTL_BOOT_SDIO1)
            bootargs = "root=/dev/mmcblk1p3 loglevel=8 rw rootdelay=4 rootfstype=ext4 console=ttyS0,115200 crashkernel=256M-:128M earlycon=sbi";
        else  if(g_bootmod == SYSCTL_BOOT_NORFLASH)
            //bootargs = "root=/dev/mtdblock9 rw rootwait rootfstype=jffs2 console=ttyS0,115200 earlycon=sbi";
            //bootargs = "ubi.mtd=9 rootfstype=ubifs rw root=ubi0_0 console=ttyS0,115200 earlycon=sbi";
            bootargs = "ubi.mtd=9 rootfstype=ubifs rw root=ubi0_0 console=ttyS0,115200 earlycon=sbi fw_devlink=off quiet";
    }
    //printf("%s\n",bootargs);
    return bootargs;
}
#endif 
static int k230_boot_decomp_to_load_addr(image_header_t *pUh, ulong des_len, ulong data , ulong *plen)
{
    int ret = 0;
    K230_dbg("imge: %s load to %x  compress =%x  src %lx len=%lx \n", image_get_name(pUh), image_get_load(pUh), image_get_comp(pUh), data, *plen);    

    //设计要求必须gzip压缩，调试可以不压缩；
    if (image_get_comp(pUh) == IH_COMP_GZIP) {        
        ret = gunzip((void *)(ulong)image_get_load(pUh), des_len, (void *)data, plen);
        if(ret){
            printf("unzip fialed ret =%x\n", ret);
            return -1;
        }
    }else if(image_get_comp(pUh) == IH_COMP_NONE){
        memmove((void *)(ulong)image_get_load(pUh), (void *)data, *plen);
    }
    flush_cache(image_get_load(pUh), *plen);
    K230_dbg("imge: %s load to %x  compress =%x  src %lx len=%lx  \n", image_get_name(pUh), image_get_load(pUh), image_get_comp(pUh), data, *plen);    
    return ret;
}
static int  de_reset_big_core(ulong core_run_addr)
{
    /*
    p/x *(uint32_t*)0x9110100c
    set  *(uint32_t*)0x9110100c=0x10001000    //清 done bit
    set *(uint32_t*)0x9110100c=0x10001       //设置 reset bit
    p/x *(uint32_t*)0x9110100c
    set   *(uint32_t*)0x9110100c=0x10000       //清 reset bit
    p/x *(uint32_t*)0x9110100c
    */
    writel(core_run_addr, (void*)0x91102104ULL);//cpu1_hart_rstvec 设置大核的解复位向量，复位后程序执行位置；
    //printf("0x91102104 =%x 0x9110100c=%x\n", readl( (void*)0x91102104ULL), readl( (void*)0x9110100cULL));

    //writel(0x80199805, (void*)0x91100004); //1.6Ghz

    writel(0x10001000, (void*)0x9110100cULL); //清 done bit
    writel(0x10001, (void*)0x9110100cULL); //设置 reset bit
    //printf("0x9110100c =%x\n", readl( (void*)0x9110100cULL));    
    writel(0x10000, (void *)0x9110100cULL); ////清 reset bit  
    //printf("0x9110100c =%x\n", readl( (void*)0x9110100cULL));
    //printf("reset big hart\n");
    return 0;
}
/**
 * @brief 
 * 
 * @param pUh  image_header_t *
 * @return int 
 */
static int k230_boot_rtt_uimage(image_header_t *pUh)
{
    int ret = 0;
    //小核是0，大核是1；
    ulong len = image_get_size(pUh);
    ulong data;

    image_multi_getimg(pUh, 0, &data, &len);
    ret = k230_boot_decomp_to_load_addr(pUh, 0x6000000, data, &len );
    if( ret == 0){
        de_reset_big_core(image_get_load(pUh));
    }
    return 0;
}

static int k230_boot_linux_uimage(image_header_t *pUh)
{
    int ret = 0;
    void (*kernel)(ulong hart, void *dtb);

    //小核是0，大核是1；
    ulong len = image_get_size(pUh);
    ulong data;
    ulong dtb;
    ulong rd,rd_len;
    ulong img_load_addr = 0;


    //record_boot_time_info("gd");
    image_multi_getimg(pUh, 0, &data, &len);
    img_load_addr = (ulong)image_get_load(pUh);

    ret = k230_boot_decomp_to_load_addr(pUh, (ulong)pUh-img_load_addr,  data, &len );
    if( ret == 0){
          //dtb
        image_multi_getimg(pUh, 2, &dtb, &len);
        image_multi_getimg(pUh, 1, &rd, &rd_len);
        #ifdef USE_UBOOT_BOOTARGS
        len = fdt_shrink_to_minimum((void*)dtb,0x100);
        ret = fdt_chosen((void*)dtb);
        #endif 
        memmove((void*)OPENSBI_DTB_ADDR, (void *)dtb, len);

        #ifndef CONFIG_SPL_BUILD
        //run_command("fdt addr 0x91e7000;fdt print;", 0);
        #endif 

        if(rd_len > 0x100 )
            memmove((void*)RAMDISK_ADDR, (void *)rd, rd_len);

        K230_dbg("dtb %lx rd=%lx l=%lx  %lx %lx ci%lx %lx \n", dtb,rd, data, OPENSBI_DTB_ADDR, RAMDISK_ADDR, get_CONFIG_CIPHER_ADDR(),get_CONFIG_PLAIN_ADDR());

        cleanup_before_linux();//flush cache，
        kernel = (void (*)(ulong, void *))img_load_addr;
        //do_timeinfo(0,0,0,0); 
        kernel(0, (void*)OPENSBI_DTB_ADDR);

    }
    return ret;
}
/**
 * @brief 
 * 
 * @param pUh  image_header_t *
 * @return int 
 */
static int k230_boot_paramter_uimage(image_header_t *pUh)
{
    int ret = 0;
    ulong len = image_get_data_size(pUh);
    ulong data = image_get_data(pUh);

    ret = k230_boot_decomp_to_load_addr(pUh, 0x6000000,  data, &len );
    return ret;
}
/**
 * @brief 
 * 
 * @param pUh  image_header_t *
 * @return int 
 */
static int k230_boot_uboot_uimage(image_header_t *pUh)
{
    int ret = 0;
    void (*uboot)(ulong hart, void *dtb);
    ulong len = image_get_data_size(pUh);
    ulong data = image_get_data(pUh);


    ret = k230_boot_decomp_to_load_addr(pUh, 0x6000000,  data, &len );
    if(ret == 0){
        icache_disable();
        dcache_disable();
        // csi_l2cache_flush_invalid();
        asm volatile(".long 0x0170000b\n":::"memory");

        uboot = (void (*)(ulong, void *))(ulong)image_get_load(pUh);
        //do_timeinfo(0,0,0,0); 
        #if defined(CONFIG_LINUX_RUN_CORE_ID) && (CONFIG_LINUX_RUN_CORE_ID == 1)
        de_reset_big_core(image_get_load(pUh));
        while(1) udelay(100);
        #endif 
        uboot(0, (void*)OPENSBI_DTB_ADDR);
    }
    return 0;
}
int k230_img_boot_sys_bin(firmware_head_s * fhBUff)
{
    int ret = 0;
    image_header_t * pUh = NULL;
    ulong plain_addr  = 0;

    //解密
    //record_boot_time_info("ds");
    ret = k230_check_and_get_plain_data((firmware_head_s *)fhBUff,  &plain_addr);
    if (ret )
        return ret;

    pUh = (image_header_t *) (plain_addr + 4);//4字节是版本号
    if (!image_check_magic(pUh)){
        printf("bad magic \n");
        return -3;
    }        
    //解压缩，引导；
     if ( (0 == strcmp(image_get_name(pUh), "linux") ) || (0 == strcmp(image_get_name(pUh), "Linux") ) ){ 
        ret = k230_boot_linux_uimage(pUh);
    }else if(0 == strcmp(image_get_name(pUh), "rtt")){        
        ret = k230_boot_rtt_uimage(pUh);       
    }else if(0 == strcmp(image_get_name(pUh), "uboot")){        
        ret = k230_boot_uboot_uimage(pUh);       
    } else  {        
        k230_boot_paramter_uimage(pUh);
        return 0;
    }
    return 0;
}

static int k230_check_and_get_plain_data_securiy(firmware_head_s *pfh, ulong *pplain_addr)
{
    int ret = 0;
    pufs_dgst_st md;
    char * pplaint=(char *)CONFIG_PLAIN_ADDR; //明文、0x0 0x8000000 0x0 0x7fff000
    unsigned int outlen;
    uint8_t puk_hash_otp[32];
    uint8_t temp32_0[32]={0};

    const char *gcm_iv = "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94";
    const char *sm4_iv = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";
    const char *gcm_key = "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f";
    pufs_ec_point_st puk;
    pufs_ecdsa_sig_st sig;

    if(pfh->crypto_type == INTERNATIONAL_SECURITY) {
        K230_dbg(" INTERNATIONAL_SECURITY aes \n");
        // 检验头携带的RSA2048/SM2的 public key是否正确，与烧录到OTP里面的PUK HASH值做比对。
        // 直接把public key烧录到OTP也可以，但是需要消耗2Kbit的OTP空间，这里主要是节约otp空间考虑。
        // 把PUK HASH烧录到OTP，可以保证只有经过PRK签名的固件才可以正常启动。
        if(SUCCESS != cb_pufs_read_otp(puk_hash_otp, 32, OTP_BLOCK_RSA_PUK_HASH_ADDR)){
            printf("otp read puk hash error \n");
            return -6;
        }
        if(0 == memcmp(temp32_0, puk_hash_otp,32)){
            printf("you have not input otp key,not support security boot\n");
            return -18;
        }
        if (cb_pufs_hash(&md, (const uint8_t*)&pfh->verify, 256 + 4, SHA_256) != SUCCESS){
            printf("hash calc error \n");
            return -7;
        }
        //验证公钥是否正确
        if(  memcmp(md.dgst, puk_hash_otp, 32)){
            printf("pubk hash error \n");
            return -8;
        }

        //使用公钥验证mac签名
        char *gcm_tag = (char *)(pfh+1) + pfh->length - 16;
        ret = pufs_rsa_p1v15_verify(pfh->verify.rsa.signature, 
                                    RSA2048, 
                                    pfh->verify.rsa.n, 
                                    pfh->verify.rsa.e, 
                                    gcm_tag, 
                                    16);
        if (ret ) {
            printf("rsa verify error \n");
            return -9;
        } 

        //gcm解密,同时保证完整性；获取明文        
        ret = pufs_dec_gcm((uint8_t *)pplaint, &outlen, (const uint8_t *)(pfh+1), pfh->length - 16,
             AES, OTPKEY, OTPKEY_2, 256, (const uint8_t *)gcm_iv, 12, NULL, 0, gcm_tag, 16);
        if (ret )  {
            printf("dec gcm error ret=%x \n",ret);
            return -10;
        }            

        //pUimgh = (image_header_t *)(pplaint);    
        if(pplain_addr) 
            *pplain_addr = (ulong)pplaint;
        
    }else if (pfh->crypto_type == CHINESE_SECURITY){
        K230_dbg("CHINESE_SECURITY sm\n");
        if(SUCCESS != cb_pufs_read_otp(puk_hash_otp, 32, OTP_BLOCK_SM2_PUK_HASH_ADDR)){
            printf("otp read puk hash error \n");
            return -6;
        }
         if(0 == memcmp(temp32_0, puk_hash_otp,32)){
            printf("you have not input otp key,not support security boot\n");
            return -18;
        }
        if (cb_pufs_hash(&md, (const uint8_t *)&pfh->verify, 512 + 4 - 32 - 32, SM3) != SUCCESS){
            printf("hash calc error \n");
            return -7;
        }
        //验证公钥是否正确
        if(memcmp(md.dgst, puk_hash_otp, 32)){
            printf("pubk hash error \n");
            return -8;
        }

        //SM2 解密hash验签        
        puk.qlen = sig.qlen = 32;
        memcpy(puk.x, pfh->verify.sm2.pukx, puk.qlen);
        memcpy(puk.y, pfh->verify.sm2.puky, puk.qlen);
        memcpy(sig.r, pfh->verify.sm2.r, sig.qlen);
        memcpy(sig.s, pfh->verify.sm2.s, sig.qlen);
        if(cb_pufs_sm2_verify(sig,  (const uint8_t *)(pfh+1), pfh->length, 
                                pfh->verify.sm2.id, pfh->verify.sm2.idlen, puk) != SUCCESS){
            printf("sm verify error\n");                                    
            return -11;
        }

        //SM4 CBC 解密
        if(cb_pufs_dec_cbc((uint8_t *)pplaint, &outlen, (const uint8_t *)(pfh+1), pfh->length, 
            SM4, OTPKEY, OTPKEY_4, 128, (const uint8_t *)sm4_iv, 0) != SUCCESS){
            printf("dec cbc  error\n");     
            return -12;
        } 
        if(pplain_addr) 
            *pplain_addr = (ulong)pplaint;     
    }
    else if(pfh->crypto_type == GCM_ONLY) {
        K230_dbg(" POC GCM_ONLY \n");
        char *gcm_tag = (char *)(pfh+1) + pfh->length - 16;

        //gcm解密,同时保证完整性；获取明文        
        ret = pufs_dec_gcm_poc((uint8_t *)pplaint, &outlen, (const uint8_t *)(pfh+1), pfh->length - 16,
             AES, SSKEY, (const uint8_t *)gcm_key, 256, (const uint8_t *)gcm_iv, 12, NULL, 0, (uint8_t *)gcm_tag, 16);
        if (ret )  {
            printf("dec gcm error ret=%x \n",ret);
            return -10;
        }

        if(pplain_addr) 
            *pplain_addr = (ulong)pplaint;
    }           
    else 
        return -10;

    return 0;
}

//去掉k230 fireware头信息，完整性校验，解密；
static int k230_check_and_get_plain_data(firmware_head_s *pfh, ulong *pplain_addr)
{

    uint32_t otp_msc = 0;    
    int ret = 0;
    pufs_dgst_st md;

    if(pfh->magic != MAGIC_NUM){
        printf("magic error %x : %x \n", MAGIC_NUM, pfh->magic);
        return CMD_RET_FAILURE;
    }

    if(pfh->crypto_type == NONE_SECURITY){ 
        //printf(" NONE_SECURITY \n");
        if(SUCCESS != cb_pufs_read_otp((uint8_t *)&otp_msc, OTP_BLOCK_PRODUCT_MISC_BYTES, OTP_BLOCK_PRODUCT_MISC_ADDR)){
            printf("otp read error \n");
            return -4;
        }
        if(otp_msc & 0x1){
            printf(" NONE_SECURITY not support  %x \n", pfh->crypto_type);
            return -5;
        }       
        //校验完整性
        #ifdef  CONFIG_K230_PUFS
		cb_pufs_hash(&md, (const uint8_t*)(pfh + 1), pfh->length, SHA_256);
        #else 
        sha256_csum_wd((const uint8_t*)(pfh + 1), pfh->length, md.dgst, CHUNKSZ_SHA256);
        #endif   

        if(memcmp(md.dgst, pfh->verify.none_sec.signature, SHA256_SUM_LEN) ){
            printf("sha256 error ");
            return -3; 
        }   
            

        if(pplain_addr) 
            *pplain_addr = (ulong)pfh + sizeof(*pfh) ; 

        ret = 0;    
	} else if((pfh->crypto_type  == CHINESE_SECURITY)|| (pfh->crypto_type  == INTERNATIONAL_SECURITY) || (pfh->crypto_type  == GCM_ONLY))
        ret = k230_check_and_get_plain_data_securiy(pfh, pplain_addr);
    else  {
        printf("error crypto type =%x\n", pfh->crypto_type);
        return -9;
    } 
    return ret;
}

//mmc


#ifdef SUPPORT_MMC_LOAD_BOOT
static ulong get_blk_start_by_boot_firmre_type(en_boot_sys_t sys)
{
    ulong blk_s = IMG_PART_NOT_EXIT;
    switch (sys){
	case BOOT_SYS_LINUX:
		blk_s = LINUX_SYS_IN_IMG_OFF_SEC;
		break;
	case BOOT_SYS_RTT:
		blk_s = RTT_SYS_IN_IMG_OFF_SEC;
		break;
    case BOOT_SYS_UBOOT:
		blk_s = CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR;
		break;
    default:break;
	} 
    return blk_s;
}

//dev ,linux ,buff
//sysctl_boot_mode_e bmode, en_boot_sys_t sys, ulong buff
static int k230_load_sys_from_mmc_or_sd(en_boot_sys_t sys, ulong buff)//(ulong offset ,ulong buff)
{
    static struct blk_desc *pblk_desc = NULL;
    ulong blk_s  = get_blk_start_by_boot_firmre_type(sys);
    struct mmc * mmc=NULL;
    int ret = 0;
    firmware_head_s *pfh = (firmware_head_s *)buff;
    ulong data_sect = 0;

    if( IMG_PART_NOT_EXIT == blk_s )
        return IMG_PART_NOT_EXIT;

    if(NULL == pblk_desc){
        if(mmc_init_device(g_bootmod - SYSCTL_BOOT_SDIO0)) //mmc_init_device
            return 1;

        mmc= find_mmc_device( g_bootmod - SYSCTL_BOOT_SDIO0 );
        if(NULL == mmc)
            return 2;
        if(mmc_init(mmc))
            return 3;

        pblk_desc = mmc_get_blk_desc(mmc);
        if(NULL == pblk_desc)
            return 3;
    }

    ret = blk_dread(pblk_desc, blk_s , HD_BLK_NUM, (char *)buff);
    if(ret != HD_BLK_NUM)
        return 4;

    if(pfh->magic != MAGIC_NUM){
        K230_dbg("pfh->magic 0x%x != 0x%x blk=0x%lx buff=0x%lx  ", pfh->magic, MAGIC_NUM, blk_s, buff);
        return 5;
    }

    data_sect = DIV_ROUND_UP(pfh->length + sizeof(*pfh), BLKSZ) - HD_BLK_NUM;
	
    ret = blk_dread(pblk_desc, blk_s + HD_BLK_NUM, data_sect, (char *)buff + HD_BLK_NUM * BLKSZ);
    if(ret != data_sect)
        return 6;
    return 0;
}

#endif  //SUPPORT_MMC_LOAD_BOOT
static ulong get_flash_offset_by_boot_firmre_type(en_boot_sys_t sys)
{
    ulong offset = 0xffffffff;
    switch (sys){
	case BOOT_SYS_LINUX:
        #ifdef CONFIG_SPI_NOR_LK_BASE
		offset = CONFIG_SPI_NOR_LK_BASE;
        #endif 
		break;
	case BOOT_SYS_RTT:
        #ifdef CONFIG_SPI_NOR_RTTK_BASE
		offset = CONFIG_SPI_NOR_RTTK_BASE;
        #endif 
		break;
    case BOOT_QUICK_BOOT_CFG:
        #ifdef CONFIG_SPI_NOR_QUICK_BOOT_CFG_BASE
		offset = CONFIG_SPI_NOR_QUICK_BOOT_CFG_BASE;
        #endif 
		break;
    case BOOT_FACE_DB:
        #ifdef CONFIG_SPI_NOR_FACE_DB_CFG_BASE
		offset = CONFIG_SPI_NOR_FACE_DB_CFG_BASE;
        #endif 
		break;
    case BOOT_SENSOR_CFG:
        #ifdef  CONFIG_SPI_NOR_SENSOR_CFG_CFG_BASE
		offset = CONFIG_SPI_NOR_SENSOR_CFG_CFG_BASE;
        #endif 
		break;
    case BOOT_AI_MODE:
        #ifdef CONFIG_SPI_NOR_AI_MODE_CFG_BASE
		offset = CONFIG_SPI_NOR_AI_MODE_CFG_BASE;
        #endif 
		break;
    case BOOT_SPECKLE:
        #ifdef CONFIG_SPI_NOR_SPECKLE_CFG_BASE
		offset = CONFIG_SPI_NOR_SPECKLE_CFG_BASE;
        #endif 
		break;
    case BOOT_RTAPP:
        #ifdef CONFIG_SPI_NOR_RTT_APP_BASE
		offset = CONFIG_SPI_NOR_RTT_APP_BASE;
        #endif 
		break;
    case BOOT_SYS_UBOOT:
		offset = CONFIG_SYS_SPI_U_BOOT_OFFS;
		break;
    default:break;
	} 
    return offset;
}
static int k230_load_sys_from_spi_nor( en_boot_sys_t sys, ulong buff)
{
    //g_bootmod
    int ret = 0;
    static struct spi_flash *boot_flash=NULL;
    struct udevice *new, *bus_dev;

    firmware_head_s *pfh = (firmware_head_s *)buff;
    ulong off = get_flash_offset_by_boot_firmre_type(sys);//(sys == BOOT_SYS_LINUX) ? LINUX_SYS_IN_SPI_NOR_OFF : RTT_SYS_IN_SPI_NOR_OFF;

    if(boot_flash == NULL){
        /* speed and mode will be read from DT */
        ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
                        &new);
        if (ret) {
            env_set_default("spi_flash_probe_bus_cs() failed", 0);
            return ret;
        }
        boot_flash = dev_get_uclass_priv(new);
    }


    ret = spi_flash_read(boot_flash, off, sizeof(*pfh), (void *)pfh);

    if(ret || (pfh->magic != MAGIC_NUM) )   {
        K230_dbg("pfh->magic 0x%x != 0x%x off=0x%lx buff=0x%lx  ", pfh->magic, MAGIC_NUM, off, buff);
        return 5;
    }
    ret = spi_flash_read(boot_flash, off+sizeof(*pfh), round_up(pfh->length,2), (void*)(buff+sizeof(*pfh)));

    if(sys== BOOT_SYS_LINUX){
        ret = spi_find_bus_and_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, &bus_dev, &new);
        device_remove(new, DM_REMOVE_NORMAL);
        boot_flash = NULL;
     }
    return ret;
}
static ulong get_nand_start_by_boot_firmre_type(en_boot_sys_t sys)
{
    ulong blk_s = IMG_PART_NOT_EXIT;
    switch (sys){
	case BOOT_SYS_LINUX:
		blk_s = LINUX_SYS_IN_SPI_NAND_OFF;
		break;
	case BOOT_SYS_RTT:
		blk_s = RTT_SYS_IN_SPI_NAND_OFF;
		break;
    case BOOT_SYS_UBOOT:
		blk_s = CONFIG_SYS_SPI_U_BOOT_OFFS;
		break;
    default:break;
	} 
    return blk_s;
}
static int k230_load_sys_from_spi_nand( en_boot_sys_t sys, ulong buff)
{
    //g_bootmod
    int ret = 0;
    static struct mtd_info *boot_flash=NULL;
    struct udevice *new, *bus_dev;

    firmware_head_s *pfh = (firmware_head_s *)buff;
    ulong off = get_nand_start_by_boot_firmre_type(sys); //(sys == BOOT_SYS_LINUX) ? LINUX_SYS_IN_SPI_NAND_OFF : RTT_SYS_IN_SPI_NAND_OFF;
    size_t len = sizeof(*pfh);
    size_t lenth ;

    if( IMG_PART_NOT_EXIT == off)
        return IMG_PART_NOT_EXIT;

    if(boot_flash == NULL){
        /* speed and mode will be read from DT */
        ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
                        &new);
        if (ret) {
            env_set_default("spi_flash_probe_bus_cs() failed", 0);
            return ret;
        }
        boot_flash = dev_get_uclass_priv(new);
    }

    ret = nand_read(boot_flash, off, &len, (void *)pfh);

    if(ret || (pfh->magic != MAGIC_NUM) ){
        K230_dbg("pfh->magic 0x%x != 0x%x off=0x%lx buff=0x%lx ", pfh->magic, MAGIC_NUM, off, buff);
        return 5;
    }

    lenth = round_up(pfh->length,2);
    ret = nand_read(boot_flash, off+sizeof(*pfh), &lenth, (void*)(buff+sizeof(*pfh)));
    if(sys== BOOT_SYS_LINUX){
        ret = spi_find_bus_and_cs(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS, &bus_dev, &new);
        device_remove(new, DM_REMOVE_NORMAL);
        boot_flash = NULL;
     }

    return ret;
}
int k230_img_load_sys_from_dev(en_boot_sys_t sys, ulong buff)
{
    int ret = 0;
    
    if( (g_bootmod == SYSCTL_BOOT_SDIO1) || (g_bootmod == SYSCTL_BOOT_SDIO0) ){ //sd
        ret = k230_load_sys_from_mmc_or_sd(sys, buff);
    }else if(g_bootmod == SYSCTL_BOOT_NANDFLASH){ //spi nand
        ret = k230_load_sys_from_spi_nand( sys,buff);
    }else if(g_bootmod == SYSCTL_BOOT_NORFLASH){ //spi nor
        ret = k230_load_sys_from_spi_nor( sys,buff);
    }
    return ret;
}
static int k230_img_load_boot_sys_auot_boot(en_boot_sys_t sys)
{
    int ret = 0;
    #if defined(CONFIG_SPI_NOR_SUPPORT_CFG_PARAM)
    k230_img_load_boot_sys(BOOT_QUICK_BOOT_CFG);
    k230_img_load_boot_sys(BOOT_FACE_DB);
    k230_img_load_boot_sys(BOOT_SENSOR_CFG);
    k230_img_load_boot_sys(BOOT_AI_MODE);
    k230_img_load_boot_sys(BOOT_SPECKLE);
    k230_img_load_boot_sys(BOOT_RTAPP);
    #endif 


    #if  defined(CONFIG_SUPPORT_RTSMART)
    ret += k230_img_load_boot_sys(BOOT_SYS_RTT); 
    #endif  

    
    #if  defined(CONFIG_SUPPORT_RTSMART) && !defined(CONFIG_SUPPORT_LINUX)
    if(ret == 0)
        while(1) udelay(100);
    #endif 



    #if  defined(CONFIG_SUPPORT_LINUX)
    ret += k230_img_load_boot_sys(BOOT_SYS_LINUX);
    #endif 
    
    return ret;
}
/**
 * @brief 
 * 
 * @param bmode 
 * @param sys 
 * @param buff 
 * @return int 
 */
int k230_img_load_boot_sys(en_boot_sys_t sys)
{
    int ret = 0;

    if(sys == BOOT_SYS_AUTO)
        ret =  k230_img_load_boot_sys_auot_boot(sys);
    else {
        ret = k230_img_load_sys_from_dev(sys, CONFIG_CIPHER_ADDR);
        if(ret){
            if(ret != IMG_PART_NOT_EXIT)
                printf("sys %x  load error ret=%x\n", sys, ret);
            return ret;
        }
        ret = k230_img_boot_sys_bin((firmware_head_s * )CONFIG_CIPHER_ADDR);
    }
    return ret;
}

