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
#if 0
#include <pufs_hmac.h>
#include <pufs_ecc.h>
#include <pufs_rt.h>
#include <image.h>
#include <pufs_sp38d.h>
#include <pufs_ecp.h>
#include <gzip.h>
#endif 

#define MAGIC_NUM   0x3033324B // "K230"

typedef enum {
    NONE_SECURITY = 0,
    CHINESE_SECURITY,
    INTERNATIONAL_SECURITY
} crypto_type_e;

typedef struct 
{
    uint32_t magic; // 方便升级时快速判断固件是否有效。
    uint32_t length; // 从存储介质读到SRAM的数据量
    crypto_type_e crypto_type; // 支持国密或国际加密算法，或支持不加密启动(otp可以控制是否支持)。
    // 设想这样一个场景，如果固件只使用对称加密，在工厂批量生产的时候，解密密钥必然会泄露给工厂。如果使用非对称加密就可以这种问题了，只需要把公钥交给工厂。
    union verify_{ 
        struct rsa_{
            uint8_t n[256];// 非对称加密的验签，防止固件被篡改。同时其HASH值会被烧录到otp。
            uint32_t e;
            uint8_t signature[256];
        } rsa;
        struct sm2_{
            uint32_t idlen;
            uint8_t id[512-32*4];
            uint8_t pukx[32];
            uint8_t puky[32];
            uint8_t r[32];
            uint8_t s[32];
        } sm2;
        struct none_sec_{
            uint8_t signature[32];// 计算HASH保证启动固件的完整性。避免程序异常难以定位原因。
            uint8_t reserved[516-32];
        } none_sec;
    } verify;
}__attribute__((packed, aligned(4))) firmware_head_s; //总的512+16 bytes


int board_init(void)
{
#ifdef CONFIG_USB_HOST
#define HS_REG (0x91585000UL)
#define USB0_TEST_CTL3 (HS_REG + 0x7c)
#define USB0_DMPULLDOWN0 (1<<8)
#define USB0_DPPULLDOWN0 (1<<9)

	u32 ctl3 = readl(USB0_TEST_CTL3);
	ctl3 |= (USB0_DMPULLDOWN0 | USB0_DPPULLDOWN0);
	writel(ctl3, USB0_TEST_CTL3);
#endif
    return 0;
}
//#define CONFIG_HAVE_HARD_UNZIP
//#define CONFIG_K230_PUFS
#ifdef CONFIG_HAVE_HARD_UNZIP
int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp){
    printf("my gunzip\n");
    return -1;    
}
#endif 

//去掉k230 fireware头信息，完整性校验，解密；
int k230_check_and_get_plain_data(firmware_head_s *pfh, ulong *pplain_addr)
{
    const char *gcm_iv = "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94";
    char * pplaint=(char *)0x6200000; //明文、98MB
    //image_header_t *pUimgh; //uimage head
    uint32_t uint32_tmp=0xa000000;
    
    int ret = 0;
    //pufs_dgst_st md;

    if(pfh->magic != MAGIC_NUM){
        printf("magic error %x : %x \n", MAGIC_NUM, pfh->magic);
        return CMD_RET_FAILURE;
    }
        
    if(pfh->crypto_type == NONE_SECURITY){        
        //校验完整性
        #ifdef  CONFIG_K230_PUFS
		ret = pufs_hash(&md, (const uint8_t*)(pfh + 1), pfh->length, SHA_256);
        #else 
        //sha256_csum_wd((const uint8_t*)(pfh + 1), pfh->length, md.dgst, CHUNKSZ_SHA256);
        #endif   

        // if(memcmp(md.dgst, pfh->verify.none_sec.signature, SHA256_SUM_LEN) )   
        //     return -3; 

        if(pplain_addr) 
            *pplain_addr = (ulong)pfh + sizeof(*pfh) ; 

	} else  if(pfh->crypto_type == INTERNATIONAL_SECURITY) {
        #ifdef CONFIG_K230_PUFS
        //验证mac签名
        char *gcm_tag = (char *)pfh + pfh->length - 16;
        ret = pufs_rsa_p1v15_verify(pfh->verify.rsa.signature, 
                                    RSA2048, 
                                    pfh->verify.rsa.n, 
                                    pfh->verify.rsa.e, 
                                    gcm_tag, 
                                    16);
        if (ret )  
            return CMD_RET_FAILURE;

        //gcm解密,同时保证完整性；获取明文        
        ret = pufs_dec_gcm((uint8_t *)pplaint, &uint32_tmp, (const uint8_t *)(pfh+1), pfh->length - 16,
             AES, OTPKEY, OTPKEY_2, 256, (const uint8_t *)gcm_iv, 12, NULL, 0, gcm_tag, 16);
        if (ret )  
            return CMD_RET_FAILURE;      

        #endif           

       
        //pUimgh = (image_header_t *)(pplaint);    
        if(pplain_addr) 
            *pplain_addr = (ulong)pplaint;
    }
    return 0;
}

/**
 * @brief  k230 load image
 *  密文镜像从sd/emmc/flash/mem加载到buff
 * 
 * @param argc 
 * @param argv 
 * @param buff 
 * @return int 
 */
int k230_load_image(int argc, char *const argv[],  ulong  buff)
{
    //char * pcipher=0x6600000; //加载地址、密文
    ulong len,add_tmp;
    firmware_head_s *pfh = (firmware_head_s *)buff;

    if(argc < 2)
        return CMD_RET_USAGE;

    if(!strcmp(argv[1], "sd")){
        //以sd卡为例；
        //mmc read xx xx; 读头；load_add
        //mmc dev 0
        //mmc read 0x6600000  0 1 //仅读部分头

        if(pfh->magic != MAGIC_NUM){
            printf("magic error %x : %x \n", MAGIC_NUM, pfh->magic);
            return CMD_RET_FAILURE;
        }
        //读密文数据 //mmc read xx  pload_buf 
        //mmc read 0x6600000+512 1 100  (pfh->length + sizeof(firmware_head_s) ) /512 -1;  
    }else if(!strcmp(argv[1], "mem")) {
        if(argc < 4)
            return CMD_RET_USAGE;
        add_tmp = simple_strtoul(argv[2],NULL, 0);
        len = simple_strtoul(argv[3],NULL, 0);
        if(add_tmp != buff){
            memmove((void *)buff, (void *)add_tmp, len);
        }
    }else if ("spi nor")
    {
        /* code */
    }else if ("spi nand")
    {
        /* code */
    }
    return 0;

}
/**
 * @brief 
 * 
 * @param pUh  image_header_t *
 * @return int 
 */
int k230_boot_rtt(image_header_t *pUh)
{
    int ret = 0;
    //小核是0，大核是1；
    ulong len = image_get_size(pUh);
    ulong data;

    image_multi_getimg(pUh, 0, &data, &len);

    //设计要求必须gzip压缩，调试可以不压缩；
    if (image_get_comp(pUh) == IH_COMP_GZIP) {        
        ret = gunzip((void *)(ulong)image_get_load(pUh), 0x2000000, (void *)data, &len);
        if(ret){
            printf("unzip fialed ret =%x\n", ret);
            return -1;
        }
    }else if(image_get_comp(pUh) == IH_COMP_NONE){
        memmove((void *)(ulong)image_get_load(pUh), (void *)data, len);
    }
    /*
        p/x *(uint32_t *)0x9110100c
        set *(uint32_t *)0x9110100c=0x10001
        p/x *(uint32_t *)0x9110100c
        set *(uint32_t *)0x9110100c=0x10001000
        p/x *(uint32_t *)0x9110100c
    */
    writel(image_get_load(pUh), (void*)0x91102104ULL);//cpu1_hart_rstvec 设置大核的解复位向量，复位后程序执行位置；
    printf("0x91102104 =%x\n", readl( (void*)0x91102104ULL));

    writel(0x10001, (void*)0x9110100cULL); //复位大核
    printf("0x9110100c =%x\n", readl( (void*)0x9110100cULL));
    writel(0x10001000, (void *)0x9110100cULL); //解复位大核   
    printf("0x9110100c =%x\n", readl( (void*)0x9110100cULL));
    printf("reset big hart\n");
    
    return 0;
}
#define CONFIG_CIPHER_ADDR  0x6600000
/**
 * @brief 
 * 
 * @param cmdtp 
 * @param flag 
 * @param argc 
 * @param argv 
 * @return int 
 */
static int do_k230_boot(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
    int ret = 0;
 
    //假设：密文10MB；明文10M,解压缩后32M, ----
    //uboot--16M@112M
    //密文----10M@102M---0x6600000
    //明文----10M@98M----0x6200000
    //解压----98M@0

    ulong cipher_addr = CONFIG_CIPHER_ADDR; //加载地址、密文
    ulong plain_addr  = 0;
    image_header_t * pUh = NULL;
    char cmd[128];

    ret = k230_load_image(argc, argv, cipher_addr);
    if (ret )
        return ret;
    
    ret = k230_check_and_get_plain_data((firmware_head_s *)cipher_addr,  &plain_addr);
     if (ret )
        return ret;
    pUh = (image_header_t *) (plain_addr + 4);//4字节是版本号
    
    if ( (0 == strcmp(image_get_name(pUh), "linux") ) || (0 == strcmp(image_get_name(pUh), "Linux") )
        ){ 
        /*
            gzip  -k fw_payload.bin;
            echo a>rd;mkimage -A riscv -O linux -T multi -C gzip -a 0x3000000 -e 0x3000000 -n linux -d fw_payload.bin.gz:rd:k.dtb  a.bin
            bootm xxxx;
            ../../board/csky-ci/9xx/firmware_gen.py  -i a.bin -o aa.bin -n
            restore aa.bin  binary  0x5000000
            k230_boot  mem 0x5000000   0x5204db
        */   
        sprintf(cmd,"bootm 0x%lx -", (ulong)pUh);
        ret = run_command(cmd, 0);
    }else if(0 == strcmp(image_get_name(pUh), "rtt")){        
        /*
            mkimage -A riscv -O opensbi -T multi -C gzip -a 0x40000000 -e 0x40000000 -n rtt  -d fw_payload.bin.gz  rtt.bin
            ../../board/csky-ci/9xx/firmware_gen.py  -i rtt.bin -o urtt.bin -n
            restore urtt.bin  binary  0x5000000
            k230_boot  mem 0x5000000 0x0f5982
        */
        ret = k230_boot_rtt(pUh);       
    } else  {
        printf("error %s \n", image_get_name(pUh));
        return 3;
    }

    return ret;
}
/*
boot sd/mmc/spinor/spinand/mem  add 
先实现从sd启动吧；
*/
U_BOOT_CMD_COMPLETE(
	k230_boot, 4, 0, do_k230_boot,
	"boot <sd/mmc/spinor/spinand/mem>  <add|offset> [len]",
	"boot <sd/mmc/spinor/spinand/mem>  <add|offset> [len]", NULL
);



