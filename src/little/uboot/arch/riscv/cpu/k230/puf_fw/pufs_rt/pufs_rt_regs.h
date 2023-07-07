/**
 * @file      pufs_rt_regs.h
 * @brief     PUFsecurity PUFrt register definition
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

#ifndef __PUFS_RT_REGS_H__
#define __PUFS_RT_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------MEMORY MAPPED REGISTER SETTING----------------------//
extern size_t PUFRT_ADDR_START;
extern size_t PUFRT_MAP_SIZE;

//----------------------REGISTER SETTINGS-----------------------------------//
#define PIF_LEN 64
#define PUF_LEN 64
#define PTR_PTC_LEN 16
#define PIF_RWLCK_START_INDEX 32

//----------------------REGISTER STRUCT-------------------------------------//

struct pufs_rt_regs
{
    
    volatile uint32_t pif[64];
    uint32_t _pad1[64];
    volatile uint32_t ptr[16];
    volatile uint32_t ptc[16];
#ifdef PSIOT_012CW01D_B12C
    volatile uint32_t ptm[2];
    uint32_t _pad2[6];
#else
    uint32_t _pad2[8];
#endif
    volatile uint32_t rn;
    volatile uint32_t rn_status;
    volatile uint32_t healthcfg;
    volatile uint32_t feature;
    volatile uint32_t interrupt;
    volatile uint32_t otp_psmsk[2];
    volatile uint32_t puf_psmsk; // puf_psmsk & lck_psmsk
    volatile uint32_t version;
    volatile uint32_t status;
    volatile uint32_t cfg;
    volatile uint32_t set_pin;
    volatile uint32_t auto_repair;
    volatile uint32_t ini_off_chk;
    volatile uint32_t repair_pgn;
    volatile uint32_t repair_reg;
    volatile uint32_t puf_qty_chk;
    volatile uint32_t puf_enroll;
    volatile uint32_t puf_zeroize;
    volatile uint32_t set_flag;
    volatile uint32_t otp_zeroize;
    uint32_t _pad3[3];
    volatile uint32_t puf[64];
    volatile uint32_t otp[256];
};

//----------------------REGISTER BITS---------------------------------------//
#define PIF_00_SHFWEN_BITS               0
#define PIF_00_SHFREN_BITS               4
#define PIF_00_PUFLCK_BITS               8
#define PIF_00_TMLCK_BITS               12
#define PIF_00_ENROLL_BITS              16
#define PIF_00_OTPLCK_BITS              20
#define PIF_00_REPAIR_BITS              24
#define PIF_00_PGMPRT_BITS              28

//----------------------REGISTER BIT MASKS----------------------------------//
#define PIF_00_SHFWEN_MASK              0x0000000f
#define PIF_00_SHFREN_MASK              0x000000f0
#define PIF_00_PUFLCK_MASK              0x00000f00
#define PIF_00_TMLCK_MASK               0x0000f000
#define PIF_00_ENROLL_MASK              0x000f0000
#define PIF_00_OTPLCK_MASK              0x00f00000
#define PIF_00_REPAIR_MASK              0x0f000000
#define PIF_00_PGMPRT_MASK              0xf0000000
#define PIF_04_SECR_CTL_MASK            0x00ffffff
#define PIF_04_SECR_ZEROIZE_MASK        0x07000000
#define PIF_04_SECR_PROT_EN_MASK        0xf0000000
#define PIF_12_ZEROIZED_PUF_MASK        0x000000ff

#define PIF_RWLCK000_MASK               0x0000000f
#define PIF_RWLCK001_MASK               0x000000f0
#define PIF_RWLCK002_MASK               0x00000f00
#define PIF_RWLCK003_MASK               0x0000f000
#define PIF_RWLCK004_MASK               0x000f0000
#define PIF_RWLCK005_MASK               0x00f00000
#define PIF_RWLCK006_MASK               0x0f000000
#define PIF_RWLCK007_MASK               0xf0000000

#define PIF_RWLCK000_RO                 0x0000000c
#define PIF_RWLCK001_RO                 0x000000c0
#define PIF_RWLCK002_RO                 0x00000c00
#define PIF_RWLCK003_RO                 0x0000c000
#define PIF_RWLCK004_RO                 0x000c0000
#define PIF_RWLCK005_RO                 0x00c00000
#define PIF_RWLCK006_RO                 0x0c000000
#define PIF_RWLCK007_RO                 0xc0000000

#define PIF_RWLCK000_NA                 0x00000000
#define PIF_RWLCK001_NA                 0x00000000
#define PIF_RWLCK002_NA                 0x00000000
#define PIF_RWLCK003_NA                 0x00000000
#define PIF_RWLCK004_NA                 0x00000000
#define PIF_RWLCK005_NA                 0x00000000
#define PIF_RWLCK006_NA                 0x00000000
#define PIF_RWLCK007_NA                 0x00000000

//----------------------REGISTER BIT MASKS----------------------------------//
#define RNG_HEALTH_CHK_MASK             0x00000001


//----------------------REGISTER BIT MASKS----------------------------------//
#define CFG_FEATURES_RN_MASK            0x00000003
#define CFG_FEATURES_UID_MASK           0x00000300
#define CFG_FEATURES_UIDD_MASK          0x00000400
#define CFG_FEATURES_HMC_MASK           0x00030000
#define CFG_FEATURES_DR_MASK            0x01000000
#define CFG_FEATURES_SHF_MASK           0x02000000
#define CFG_FEATURES_RR_MASK            0x04000000
#define CFG_FEATURES_RB_MASK            0x08000000
#define CFG_FEATURES_LDO_MASK           0x10000000
#define CFG_FEATURES_EOFF_MASK          0x20000000

//----------------------REGISTER BIT MASKS----------------------------------//
#define INT_INTRPT_ST_PS_MASK           0x00000001
#define INT_INTPRT_ST_OP_MASK           0x00000002
#define INT_INTPRT_ST_IP_MASK           0x00000004
#define INT_INTPRT_ST_MS_MASK           0x00000008
#define INT_INTPRT_EN_PS_MASK           0x00010000
#define INT_INTPRT_EN_OP_MASK           0x00020000
#define INT_INTPRT_EN_IP_MASK           0x00040000
#define INT_INTPRT_EN_MS_MASK           0x00080000

//----------------------REGISTER BIT MASKS----------------------------------//
#define PMK_PUF_PSMSK_MASK               0x000000FF
#define PMK_LCK_PSMSK_MASK               0xFF000000

//----------------------REGISTER BIT MASKS----------------------------------//
#define PTM_STATUS_BUSY_MASK            0x00000001
#define PTM_STATUS_ERROR_MASK           0x00000002
#define PTM_STATUS_WARNING_MASK         0x00000004
#define PTM_STATUS_WRONG_MASK           0x00000008
#define PTM_STATUS_FORBID_MASK          0x00000010
#define PTM_STATUS_FAILCNT_MASK         0x00000f00
#define PTM_CFG_REG_PDSTB_MASK          0x00000001
#define PTM_CFG_REG_PTR_PTC_MASK        0x00000002
#define PTM_CFG_REG_PGM_IGN_MASK        0x00000004
#define PTM_CFG_REG_PVPRRDY_MASK        0x00000008
#define PTM_CFG_REG_RD_MODE_MASK        0x00000030
#define PTM_CFG_REG_RD_TO_SEL_MASK      0x000000C0
#define PTM_CFG_REG_FRE_COUNT_MASK      0x00000100
#define PTM_CFG_REG_RNG_CONT_MASK       0x00000200
#define PTM_CFG_REG_DIS_DR_MASK         0x00000400

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_RT_REGS_H__*/
