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
#include<stdio.h>
#include <common.h>
#include "platform.h"
#include <asm/io.h>
#include <cpu_func.h>
#include <command.h>
#include <linux/delay.h>
#include <malloc.h>
#include <gzip.h>
// #pragma GCC push_options
// #pragma GCC optimize ("O0")

#define ZIP_LINE_SIZE           (128 * 1024)
#define ZIP_RD_CH               DMA_CH_0
#define ZIP_WR_CH               DMA_CH_1
#define SRAM_RD_ADDR            (0x80280000)        // mem --> sram
#define SRAM_WR_ADDR            (0X80200000)        // sram --> mem
#define SDMA_CH_LENGTH          0x30

typedef struct sdma_llt {
    uint32_t reserved_0 : 28;
    uint32_t dimension : 1;
    uint32_t pause : 1;
    uint32_t node_intr : 1;
    uint32_t reserved : 1;

    uint32_t src_addr;

    uint32_t line_size;

    uint32_t line_num : 16;
    uint32_t line_space : 16;

    uint32_t dst_addr;

    uint32_t next_llt_addr;
} sdma_llt_t;

typedef enum DIMENSION {
    DIMENSION1,
    DIMENSION2,
} sdma_dimension_e;

struct ugzip_reg {
    uint32_t decomp_start;
    uint32_t gzip_src_size;
    uint32_t dma_out_size;
    uint32_t decomp_intstat;
};
typedef enum dma_ch {
    DMA_CH_0 = 0,
    DMA_CH_1 = 1,
    DMA_CH_2 = 2,
    DMA_CH_3 = 3,
    DMA_CH_4 = 4,
    DMA_CH_5 = 5,
    DMA_CH_6 = 6,
    DMA_CH_7 = 7,
    DMA_CH_MAX,
} dma_ch_t;

typedef enum ugzip_rw {
    UGZIP_RD = 0,
    UGZIP_WR = 1,
} ugzip_rw_e;

typedef struct sdma_ch_cfg {
    uint32_t ch_ctl;
    uint32_t ch_status;
    uint32_t ch_cfg;
    uint32_t ch_usr_data;
    uint32_t ch_llt_saddr;
    uint32_t ch_current_llt;
} sdma_ch_cfg_t;

typedef struct gsdma_ctrl {
    uint32_t dma_ch_en;
    uint32_t dma_int_mask;
    uint32_t dma_int_stat;
    uint32_t dma_cfg;
    uint32_t reserved[11];
    uint32_t dma_weight;
} gsdma_ctrl_t;



sdma_llt_t *g_llt_list_w=NULL;
sdma_llt_t *g_llt_list_r=NULL;
static uint32_t *ugzip_llt_cal(uint8_t *addr, uint32_t length, ugzip_rw_e mode)
{
    int i;
    uint32_t list_num;
    sdma_llt_t *llt_list;

    list_num = (length - 1) / ZIP_LINE_SIZE + 1;
    llt_list = (sdma_llt_t *)malloc(sizeof(sdma_llt_t)*list_num);
    if(NULL  == llt_list){
        printf("malloc error =\n" );
        return NULL;
    }

    if (mode == UGZIP_RD) {
        g_llt_list_r = llt_list ;//= (sdma_llt_t *)(uint64_t)0x80300000;
    } else {
        g_llt_list_w = llt_list;// = (sdma_llt_t *)(uint64_t)0x80310000;
    }


    // if (mode == UGZIP_RD) {
    //     llt_list = (sdma_llt_t *)(uint64_t)0x80300000;
    // } else {
    //     llt_list = (sdma_llt_t *)(uint64_t)0x80310000;
    // }
    memset(llt_list, 0, sizeof(sdma_llt_t)*list_num);
    for (i = 0; i < list_num; i++) {
        llt_list[i].dimension = DIMENSION1;
        llt_list[i].pause = 0;
        llt_list[i].node_intr = 0;

        if (mode == UGZIP_RD) {             /* from memory to sram */
            llt_list[i].src_addr = ((uint32_t)(uint64_t)addr + ZIP_LINE_SIZE*i);
            llt_list[i].dst_addr = SRAM_RD_ADDR + (i%2) * ZIP_LINE_SIZE;
            //printf("%d, i:%d, llt_list[i].src_addr: %08x, llt_list[i].dst_addr:     %08x\r\n", __LINE__, i, llt_list[i].src_addr, llt_list[i].dst_addr);
        } else if (mode == UGZIP_WR) {      /* from sram to memory */
            llt_list[i].src_addr = SRAM_WR_ADDR + (i%4) * ZIP_LINE_SIZE;
            llt_list[i].dst_addr = ((uint32_t)(uint64_t)addr + ZIP_LINE_SIZE*i);
            //printf("%d, i:%d, llt_list[i].src_addr: %08x, llt_list[i].dst_addr:     %08x\r\n", __LINE__, i, llt_list[i].src_addr, llt_list[i].dst_addr);
        }
        
        if (i == list_num - 1) {
            // llt_list[i].line_size = length % ZIP_LINE_SIZE;
            llt_list[i].line_size = ZIP_LINE_SIZE;
            llt_list[i].next_llt_addr = 0;
            //printf("%d, i:%d, llt_list[i].line_size:%08x, llt_list[i].next_llt_addr:%08x\r\n", __LINE__, i, llt_list[i].line_size, llt_list[i].next_llt_addr);
        } else {
            llt_list[i].line_size = ZIP_LINE_SIZE;
            llt_list[i].next_llt_addr = (uint32_t)(uint64_t)(&llt_list[i+1]);
            //printf("%d, i:%d, llt_list[i].line_size:%08x, llt_list[i].next_llt_addr:%08x\r\n", __LINE__, i, llt_list[i].line_size, llt_list[i].next_llt_addr);
        }  

        //uint32_t *llt_printf = (uint32_t *)(&llt_list[i]);
        // printf("llt_num:%d, %08x, %08x, %08x, %08x, %08x, %08x\r\n", 
        //     i, llt_printf[0], llt_printf[1], llt_printf[2], llt_printf[3], llt_printf[4], llt_printf[5]);
    }

    flush_dcache_range((uint64_t)llt_list, (uint64_t)llt_list + sizeof(sdma_llt_t)*list_num);

    return (uint32_t *)llt_list;
}
static int ugzip_sdma_cfg(uint8_t ch, ugzip_rw_e mode, uint8_t *addr, uint32_t length)
{
    uint32_t unzip_list_add = 0;
    struct sdma_ch_cfg *ch_cfg= (struct sdma_ch_cfg *)SDMA_CH_CFG;
    struct gsdma_ctrl *gsct = (struct gsdma_ctrl *)GSDMA_CTRL_ADDR;
    uint32_t int_stat;
    

    
    writel(0x2, (volatile void   *)((uint64_t)&ch_cfg->ch_ctl + ch*SDMA_CH_LENGTH));//sdma_stop(ch);
    while(0x1 & readl((volatile void   *)((uint64_t)&ch_cfg->ch_status + ch*SDMA_CH_LENGTH)));//0x80800054 while (sdma_is_busy(ch));
    
    int_stat = readl((volatile void   *)&gsct->dma_ch_en);
    writel(int_stat|1<<ch, (volatile void   *)&gsct->dma_ch_en); //sdma_ch_enable(ch); 0x80800000
   
    writel(0x111<<ch, (volatile void   *)&gsct->dma_int_stat);//sdma_int_clear(ch, SDONE_INT | SITEM_INT | SPAUSE_INT); 0x80800008
    if (ch == ZIP_RD_CH) {
        writel(0x1 << 10, (volatile void   *)((uint64_t)&ch_cfg->ch_cfg+ ch*SDMA_CH_LENGTH));// sdma_ch_cfg[ch]->ch_cfg = (0x1 << 10); 0x80800058
    }
    unzip_list_add = (uint32_t)(uint64_t)ugzip_llt_cal(addr, length, mode);
    writel(unzip_list_add, (volatile void   *)((uint64_t)&ch_cfg->ch_llt_saddr + ch*SDMA_CH_LENGTH));
    // sdma_ch_cfg[ch]->ch_llt_saddr = (uint32_t)(uint64_t)ugzip_llt_cal(addr, length, mode); 0x80800060

    writel(0x1 , (volatile void   *)( (uint64_t)&ch_cfg->ch_ctl+ ch*SDMA_CH_LENGTH));//sdma_start(ch);0x80800050
    return 0;
}




int stand_gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp)
{
    int offset = gzip_parse_header(src, *lenp);

	if (offset < 0)
		return offset;

	return zunzip(dst, dstlen, src, lenp, 1, offset);
}


int k230_priv_unzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp){
    struct ugzip_reg  *pUgzipReg = (struct ugzip_reg  *)UGZIP_BASE_ADDR; //0x80808000ULL 
    struct sdma_ch_cfg *ch_cfg= (struct sdma_ch_cfg *)SDMA_CH_CFG;
    struct gsdma_ctrl *gsct = (struct gsdma_ctrl *)GSDMA_CTRL_ADDR;

    int ret = 3;
    uint64_t stime = get_ticks();// get_ticks();
    uint64_t etime = stime + 80000000; //1ms;
    uint32_t int_stat ,decomp_intstat;
    volatile uint32_t  *p_rest_reg = NULL;

    flush_dcache_range((uint64_t)src, (uint64_t)src + (uint64_t)*lenp);    
    //memset(dst,0xaa, dstlen);
    //flush_dcache_range((uint64_t)dst, (uint64_t)dst + dstlen);
    //printf("src %llx l=%llx d=%llx dlen=%x \n", (uint64_t)src, (uint64_t)*lenp, (uint64_t)dst, dstlen);

    writel(0x80000000, (volatile void   *)&pUgzipReg->gzip_src_size);//ugzip_reg->gzip_src_size = 0x80000000;
    /* config sdma */
    ugzip_sdma_cfg(ZIP_RD_CH, UGZIP_RD, src, *lenp);
    ugzip_sdma_cfg(ZIP_WR_CH, UGZIP_WR, dst, dstlen);

    writel(0x51f, (volatile void   *)0x91302310ULL); //*(uint32_t *)0x91302310 = 0x51f;                        // NOC bandwidth
   
    /* config ugzip */
    writel(*lenp | (0x1 << 31), (volatile void   *)&pUgzipReg->gzip_src_size);//ugzip_reg->gzip_src_size = *lenp | (0x1 << 31);
    // printf("ugzip_reg->gzip_src_size:%x, reg_value:%x\r\n", &ugzip_reg->gzip_src_size, ugzip_reg->gzip_src_size);
    writel(dstlen, (volatile void   *)&pUgzipReg->dma_out_size);//ugzip_reg->dma_out_size = dstlen;

    writel(0x3, (volatile void   *)&pUgzipReg->decomp_start);//ugzip_reg->decomp_start = 0x3; //start;
    

    do{
        int_stat = readl((volatile void   *)&gsct->dma_int_stat);
        decomp_intstat = readl((volatile void   *)&pUgzipReg->decomp_intstat);
        if(int_stat){            

            if(int_stat & 0x111)
                writel(0x111<<0, (volatile void   *)&gsct->dma_int_stat);

            if(int_stat & 0x222)
                writel(0x222, (volatile void   *)&gsct->dma_int_stat);         

            if(int_stat & 0x2) {
                if(((0x1<<10) & decomp_intstat)){
                    ret = 0;
                }else {
                    ret = 1;//crc error
                    printf("unzip crc error %x  int %x \n",decomp_intstat, int_stat);
                }
                break;
            }
        }

        if(etime < get_ticks()){
            ret =2;
            //printf("unzip time out %x pUgzipReg->decomp_intstat=%x \n", int_stat, readl((volatile void   *)&pUgzipReg->decomp_intstat));
            writel(0x111<<0, (volatile void   *)&gsct->dma_int_stat);
            writel(0x111<<1, (volatile void   *)&gsct->dma_int_stat);
            break;
        }
    }while(1);

    if(g_llt_list_r){
        free(g_llt_list_r);
        g_llt_list_r = NULL;
    }
    if(g_llt_list_w){
        free(g_llt_list_w);
        g_llt_list_w = NULL;
    }

    invalidate_dcache_range((uint64_t)dst, (uint64_t)dst + dstlen);

    etime = get_ticks();
    //printf("unzip use s %llx e %llx dif %llx\n",stime , etime, etime - stime);

    if(ret){
        //reset sdma
        p_rest_reg = (volatile int *)(0x91101000+0x54);
        writel(0x2, p_rest_reg);
        while(0 == (readl(p_rest_reg) & BIT(29)) );
        writel(BIT(29), p_rest_reg);


        //reset gzip
        p_rest_reg = (volatile int *)(0x91101000+0x5c);
        writel(0x1, p_rest_reg);
        while(0 == (readl(p_rest_reg) & BIT(31)) );
        writel(BIT(31), p_rest_reg);
    }
	/*Restore sram mapping*/
    writel(0, (volatile void   *)&pUgzipReg->gzip_src_size);
    writel(0, (volatile void   *)((uint64_t)&ch_cfg->ch_cfg));
    return ret;    
}

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp) //gunzip
{
    int ret = 0;
    int temp = 0;
    
    char *pcm=(char *)(src+2);
    if(*pcm == 0x09){  //priv gunzip
         *pcm = 0x08;
        //printf("priv gunzip\n");
        temp = le32_to_cpu(*(u32*)(src + (*lenp)-4));
        if(temp > dstlen){
            printf("dest len error %x %x \n",temp, dstlen);
            return  -4;
        }
        ret = k230_priv_unzip(dst,temp, src,lenp);  
        *lenp = temp;         
    }else{
        K230_dbg("general gunzip "); 
        ret = stand_gunzip(dst,dstlen, src,lenp);           
    }
    return ret;
}

static int do_unzip(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
    int ret = 0;
    ulong src,des,slen,dlen;

    if(argc < 4)
        return CMD_RET_USAGE;

    src = simple_strtol(argv[1],NULL, 0);    
    slen = simple_strtol(argv[2],NULL, 0); //$2 = 0x88f4
    des = simple_strtol(argv[3],NULL, 0);    
    //dlen = simple_strtol(argv[4],NULL, 0); //$2 = 0x1b29d
    dlen = le32_to_cpu(*(u32*)(src+slen-4));
    //printf(" d=%lx %lx  %lx  \n",  dlen, *(u32*)(src+slen-4), src+slen-4);

   
    ret = gunzip((void*)des, dlen, (unsigned char *)src, &slen);
    printf("ret =%x s=%lx %lx d=%lx %lx \n", ret, src, slen, des, dlen);
    return ret;
}

U_BOOT_CMD_COMPLETE(
	k230_unzip, 6, 1, do_unzip,
	"k230_unzip",
	"\n src slen dest dlen \n", NULL
);

// #pragma GCC pop_options
