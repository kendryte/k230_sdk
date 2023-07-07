/**
 * @file      pufs_ka_regs.h
 * @brief     PUFsecurity KA register definition
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

#ifndef __PUFS_KA_REGS_H__
#define __PUFS_KA_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------REGISTER SIZES--------------------------------------//
#define NUM_SK_SLOTS                    8
#define NUM_PRK_SLOTS                   3
#define NUM_SS_SLOTS                    1

//----------------------REGISTER STRUCT-------------------------------------//

struct pufs_ka_regs
{
    volatile uint32_t version;
    uint32_t _pad1[3];
    volatile uint32_t sk_free;
    volatile uint32_t pk_free;
    volatile uint32_t ss_free;
    uint32_t _pad2;
    volatile uint32_t sk[NUM_SK_SLOTS];
    volatile uint32_t pk[NUM_PRK_SLOTS];
    uint32_t _pad3[5];
    volatile uint32_t ss;
};

//----------------------REGISTER BIT MASKS----------------------------------//
// SK FREE REGISTER BIT MASKS
#define SK_FREE_PLUSE_0_MASK            0x00000001
#define SK_FREE_PLUSE_1_MASK            0x00000002
#define SK_FREE_PLUSE_2_MASK            0x00000004
#define SK_FREE_PLUSE_3_MASK            0x00000008
#define SK_FREE_PLUSE_4_MASK            0x00000010
#define SK_FREE_PLUSE_5_MASK            0x00000020
#define SK_FREE_PLUSE_6_MASK            0x00000040
#define SK_FREE_PLUSE_7_MASK            0x00000080

// PK FREE REGISTER BIT MASKS
#define PK_FREE_PLUSE_0_MASK            0x00000001
#define PK_FREE_PLUSE_1_MASK            0x00000002
#define PK_FREE_PLUSE_2_MASK            0x00000004

// SS FREE REGISTER BIT MASKS
#define SS_FREE_PLUSE_0_MASK            0x00000001

// SK REGISTER BIT MASKS
#define SK_KEY_VAILD_MASK               0x00000001
#define SK_KEY_ORIGIN_MASK              0x0000000e
#define SK_KEY_SIZE_MASK                0x00007ff0
#define SK_KEY_TAG_MASK                 0x00ff0000

// PK REGISTER BIT MASKS
#define PK_KEY_VAILD_MASK               0x00000001
#define PK_KEY_ORIGIN_MASK              0x0000000e
#define PK_KEY_SIZE_MASK                0x00007ff0

// SS REGISTER BIT MASKS
#define SS_KEY_VAILD_MASK               0x00000001
#define SS_KEY_ORIGIN_MASK              0x0000000e
#define SS_KEY_SIZE_MASK                0x00007ff0

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_KA_REGS_H__*/
