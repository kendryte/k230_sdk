/**
 * @file      pufs_ecp_regs.h
 * @brief     PUFsecurity ECP register definition
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

#ifndef __PUFS_ECP_REGS_H__
#define __PUFS_ECP_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------REGISTER STRUCT-------------------------------------//

struct pufs_ecp_regs
{
    volatile uint32_t version;
    volatile uint32_t interrupt;
    volatile uint32_t ctrl;
    volatile uint32_t status;
    volatile uint32_t err_code;
    volatile uint32_t err_pc;
    volatile uint32_t err_cmd;
    volatile uint32_t mp_version;
    uint32_t _pad1[56];
    volatile uint32_t ec;
    volatile uint32_t keysel;
    volatile uint32_t otpkba;
    volatile uint32_t key_usage;
    volatile uint32_t e_short;
    uint32_t _pad2[55];
    volatile uint32_t mac[4];
    volatile uint32_t program[64];
    volatile uint32_t data[512];
};

//----------------------REGISTER BITS---------------------------------------//

#define ECP_ECP_EC_FIELD_BITS               8
#define ECP_ECP_KEYSEL_DST_BITS             16
#define ECP_ECP_KEYSEL_SRC_1_BITS           0
#define ECP_ECP_KEYSEL_SRC_2_BITS           8

//----------------------REGISTER BIT MASKS----------------------------------//
#define ECP_INTRPT_INTRPT_ST_MASK           0x00000001
#define ECP_INTRPT_INTRPT_EN_MASK           0x00010000
#define ECP_START_START_P_MASK              0x00000001
#define ECP_STATUS_BUSY_MASK                0x00000001
#define ECP_STATUS_MPROG_MASK               0x00000002
#define ECP_STATUS_RESP_MASK                0xfffffffe
#define ECP_ECP_ERR_CODE_VALUE_MASK         0xffffffff
#define ECP_ECP_ERR_PC_VALUE_MASK           0xffffffff
#define ECP_ECP_ERR_CMD_VALUE_MASK          0xffffffff
#define ECP_ECP_EC_FIELD_MASK               0x00000f00
#define ECP_ECP_EC_H_MASK                   0x0000f000
#define ECP_ECP_KEYSEL_KEY_SEL_SRC1_MASK    0x000000ff
#define ECP_ECP_KEYSEL_KEY_SEL_SRC2_MASK    0x0000ff00
#define ECP_ECP_KEYSEL_KEY_SEL_DST_MASK     0x00ff0000
#define ECP_ECP_KEYSEL_KA_RSP_MASK          0x0f000000
#define ECP_ECP_OTPKBA_VALUE_MASK           0x000000ff
#define ECP_ECP_KEY_USAGE_VALUE_MASK        0x00000007

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_ECP_REGS_H__*/
