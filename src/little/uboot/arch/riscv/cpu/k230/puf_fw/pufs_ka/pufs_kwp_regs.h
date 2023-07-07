/**
 * @file      pufs_kwp_regs.h
 * @brief     PUFsecurity KWP register definition
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

#ifndef __PUFS_KWP_REGS_H__
#define __PUFS_KWP_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------REGISTER SIZES--------------------------------------//
#define KWP_KEY_MAXLEN 80 ///< the byte length of KWP_KEY_IN_OUT_ADDR


//----------------------REGISTER STRUCT-------------------------------------//

struct pufs_kwp_regs
{
    volatile uint32_t version;
    volatile uint32_t interrupt;
    volatile uint32_t feature;
    uint32_t _pad1;
    volatile uint32_t status;
    volatile uint32_t start;
    volatile uint32_t cfg;
    uint32_t _pad2[5];
    volatile uint32_t iv[4];
    volatile uint32_t key[KWP_KEY_MAXLEN/4];
};

//----------------------REGISTER BIT MASKS----------------------------------//
#define KWP_INTRPT_INTRPT_ST_MASK       0x00000001
#define KWP_INTRPT_INTRPT_EN_MASK       0x00010000
#define KWP_FEATURE_AES_CBC_CS2_E_MASK  0x00000001
#define KWP_FEATURE_AES_CBC_CS2_D_MASK  0x00000002
#define KWP_FEATURE_AES_KW_E_MASK       0x00000004
#define KWP_FEATURE_AES_KW_D_MASK       0x00000008
#define KWP_FEATURE_AES_KWP_E_MASK      0x00000010
#define KWP_FEATURE_AES_KWP_D_MASK      0x00000020
#define KWP_STATUS_BUSY_MASK            0x00000001
#define KWP_STATUS_RESP_MASK            0xfffffffe
#define KWP_START_START_P_MASK          0x00000001
#define KWP_CFG_CMD_MASK                0x00000003
#define KWP_CFG_CIPH_MASK               0x00000004
#define KWP_CFG_ALGO_MASK               0x000000f0
#define KWP_CFG_KEY_SIZE_MASK           0x0007ff00
#define KWP_CFG_KEY_SRC_DST_MASK        0x00080000
#define KWP_CFG_KEY_IDX_MASK            0x00f00000
#define KWP_CFG_KEK_IDX_MASK            0x0f000000
#define KWP_CFG_KEK_SIZE_MASK           0x30000000

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_KWP_REGS_H__*/
