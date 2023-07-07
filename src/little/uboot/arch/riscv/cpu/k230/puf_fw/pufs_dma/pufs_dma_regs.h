/**
 * @file      pufs_dma_regs.h
 * @brief     PUFsecurity DMA register definition
 * @copyright 2020 PUFsecurity
 */
/* THIS SOFTWARE IS SUPPLIED BY PUFSECURITY ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. TO THE FULLEST
 * EXTENT ALLOWED BY LAW, PUFSECURITY'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES,
 * IF ANY, THAT YOU HAVE PAID DIRECTLY TO PUFSECURITY FOR THIS SOFTWARE.
 */

#ifndef __PUFS_DMA_REGS_H__
#define __PUFS_DMA_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------MEMORY MAPPED REGISTER STRUCT-----------------------//
struct pufs_dma_regs
{
    volatile uint32_t version;
    volatile uint32_t interrupt;
    volatile uint32_t feature;
    uint32_t _pad1;
    volatile uint32_t status_0;
    volatile uint32_t status_1;
    uint32_t _pad2[2];
    volatile uint32_t start;
    volatile uint32_t cfg_0;
    volatile uint32_t cfg_1;
    uint32_t _pad3[2];
    volatile uint32_t dsc_cfg_0;
    volatile uint32_t dsc_cfg_1;
    volatile uint32_t dsc_cfg_2;
    volatile uint32_t dsc_cfg_3;
    volatile uint32_t dsc_cfg_4;
    uint32_t _pad4[2];
    volatile uint32_t dsc_cur_0;
    volatile uint32_t dsc_cur_1;
    volatile uint32_t dsc_cur_2;
    volatile uint32_t dsc_cur_3;
    volatile uint32_t dsc_cur_4;
    uint32_t _pad5[2];
    volatile uint32_t key_cfg_0;
    volatile uint32_t cl_cfg_0;
};

//----------------------REGISTER BIT----------------------------------------//

#define DMA_KEY_CFG_0_KEY_DST_BITS        4
#define DMA_KEY_CFG_0_KEY_SIZE_BITS       8
#define DMA_KEY_CFG_0_KEY_IDX_BITS        24

#define DMA_DSC_CFG_4_WRITE_PROT_BITS     0
#define DMA_DSC_CFG_4_READ_PROT_BITS      8
#define DMA_DSC_CFG_4_FIX_READ_BITS       16   
#define DMA_DSC_CFG_4_FIX_WRITE_BITS      17
#define DMA_DSC_CFG_4_NO_CRYP_BITS        23
#define DMA_DSC_CFG_4_OFFSET_BITS         24
#define DMA_DSC_CFG_4_DN_PAUSE_BITS       28
#define DMA_DSC_CFG_4_DN_INTRPT_BITS      29
#define DMA_DSC_CFG_4_TAIL_BITS           30
#define DMA_DSC_CFG_4_HEAD_BITS           31
//----------------------REGISTER BIT MASKS----------------------------------//
#define DMA_INTRPT_INTRPT_ST_MASK         0x00000001
#define DMA_INTRPT_INTRPT_EN_MASK         0x00010000

#define DMA_FEATURE_SGDMA_MASK            0x00000001
#define DMA_FEATURE_ASYNC_MASK            0x00000002
#define DMA_FEATURE_WIDTH_MASK            0x0000000c
#define DMA_FEATURE_RNG_MASK              0x00000010

#define DMA_STATUS_0_BUSY_MASK            0x00000001
#define DMA_STATUS_0_RESP_MASK            0xfffffffe

#define DMA_STATUS_1_VALUE_MASK           0x000000ff
#define DMA_START_START_P_MASK            0x00000001

#define DMA_CFG_0_RNG_EN_MASK             0x00000001
#define DMA_CFG_0_SG_EN_MASK              0x00000002
#define DMA_CFG_0_NO_CYPT_MASK            0x00000004

#define DMA_CFG_1_RD_DEPTH_MASK           0x000000ff
#define DMA_CFG_1_WR_DEPTH_MASK           0x0000ff00
#define DMA_CFG_1_RD_THOLD_MASK           0x00ff0000
#define DMA_CFG_1_WR_THOLD_MASK           0xff000000

#define DMA_DSC_CFG_4_OFFSET_MASK         0x0f000000
#define DMA_DSC_CFG_4_DN_PAUSE_MASK       0x10000000
#define DMA_DSC_CFG_4_DN_INTRPT_MASK      0x20000000
#define DMA_DSC_CFG_4_TAIL_MASK           0x40000000
#define DMA_DSC_CFG_4_HEAD_MASK           0x80000000

#define DMA_DSC_CFG_4_CUR_OFFSET_MASK     0x0f000000
#define DMA_DSC_CFG_4_CUR_DN_PAUSE_MASK   0x10000000
#define DMA_DSC_CFG_4_CUR_DN_INTRPT_MASK  0x20000000
#define DMA_DSC_CFG_4_CUR_TAIL_MASK       0x40000000
#define DMA_DSC_CUR_4_CUR_HEADMASK        0x80000000

#define DMA_KEY_CFG_0_KEY_SRC_MASK        0x0000000f
#define DMA_KEY_CFG_0_KEY_DST_MASK        0x000000f0
#define DMA_KEY_CFG_0_KEY_SIZE_MASK       0x0007ff00
#define DMA_KEY_CFG_0_KEY_IDX_MASK        0x1f000000

#define DMA_CL_CFG_0_CL_EN_MASK           0x00000001
#define DMA_CL_CFG_0_CL_GDLE_MASK         0x00000002
#define DMA_CL_CFG_0_CL_GKEY_MASK         0x00000004

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_DMA_REGS_H__*/
