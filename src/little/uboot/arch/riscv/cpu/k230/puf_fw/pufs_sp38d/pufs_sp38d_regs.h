/**
 * @file      pufs_sp38d_regs.h
 * @brief     PUFsecurity SP38D register definition
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

#ifndef __PUFS_SP38D_REGS_H__
#define __PUFS_SP38D_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------MEMORY MAPPED REGISTER SETTING----------------------//
extern size_t SP38D_ADDR_START;
extern size_t SP38D_MAP_SIZE;

//----------------------REGISTER STRUCT-------------------------------------//

struct pufs_sp38d_regs
{
    volatile uint32_t version;
    volatile uint32_t interrupt;
    volatile uint32_t feature;
    uint32_t _pad1;
    volatile uint32_t status;
    uint32_t _pad2;
    volatile uint32_t cfg;
    volatile uint32_t block_num;
    volatile uint32_t block_num_out;
};

//----------------------REGISTER ADDRESSES----------------------------------//
#define SP38D_VERSION_ADDR              (SP38D_ADDR_START+0x00000000)
#define SP38D_INTRPT_ADDR               (SP38D_ADDR_START+0x00000004)
#define SP38D_FEATURE_ADDR              (SP38D_ADDR_START+0x00000008)
//#define RESERVED                      (SP38D_ADDR_START+0x0000000C)
#define SP38D_STATUS_ADDR               (SP38D_ADDR_START+0x00000010)
//#define RESERVED                      (SP38D_ADDR_START+0x00000014)
#define SP38D_CFG_ADDR                  (SP38D_ADDR_START+0x00000018)

//----------------------REGISTER BITS---------------------------------------//

#define SP38C_CFG_BC_TYPE_BITS          0
#define SP38C_CFG_GHASH_BITS            2
#define SP38C_CFG_GCTR_BITS             3
#define SP38C_CFG_ENCRYPT_BITS          4
#define SP38C_CFG_REG_IN_BITS           5
#define SP38C_CFG_REG_OUT_BITS          6

//----------------------REGISTER BIT MASKS----------------------------------//
#define SP38D_INTRPT_INTRPT_ST_MASK     0x00000001
#define SP38D_INTRPT_INTRPT_EN_MASK     0x00010000
#define SP38D_FEATURE_GHASH_MASK        0x00000001
#define SP38D_FEATURE_GCTR_MASK         0x00000002
#define SP38D_STATUS_BUSY_MASK          0x00000001
#define SP38D_STATUS_RESP_MASK          0xfffffffe
#define SP38D_CFG_BC_TYPE_MASK          0x00000003
#define SP38D_CFG_GHASH_MASK            0x00000004
#define SP38D_CFG_GCTR_MASK             0x00000008
#define SP38D_CFG_ENCRYPT_MASK          0x00000010

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_SP38D_REGS_H__*/
