/**
 * @file      pufs_hmac_regs.h
 * @brief     PUFsecurity HMAC register definition
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

#ifndef __PUFS_HMAC_REGS_H__
#define __PUFS_HMAC_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

//----------------------REGISTER SIZES--------------------------------------//

#define HMAC_SW_KEY_MAXLEN      64 

//----------------------REGISTER STRUCT-------------------------------------//

struct pufs_hmac_regs
{
    volatile uint32_t version;
    volatile uint32_t interrupt;
    volatile uint32_t feature;
    uint32_t _pad1;
    volatile uint32_t status;
    uint32_t _pad2;
    volatile uint32_t cfg;
    uint32_t _pad3;
    volatile uint32_t plen;
    uint32_t _pad4[3];
    volatile uint32_t alen;
    uint32_t _pad5[3];
    volatile uint8_t  sw_key[HMAC_SW_KEY_MAXLEN];
};

//----------------------REGISTER BIT MASKS----------------------------------//
#define HMAC_HASH_INTRPT_INTRPT_ST_MASK 0x00000001
#define HMAC_HASH_INTRPT_INTRPT_EN_MASK 0x00010000
#define HMAC_HASH_FEATURE_HMAC_MASK     0x00000001
#define HMAC_HASH_FEATURE_SHA2_MASK     0x00000002
#define HMAC_HASH_FEATURE_SHA2_512_MASK 0x00000004
#define HMAC_HASH_FEATURE_SM3_MASK      0x00000008
#define HMAC_HASH_STATUS_BUSY_MASK      0x00000001
#define HMAC_HASH_STATUS_RESP_MASK      0xfffffffe
#define HMAC_HASH_CFG_VARIANT_MASK      0x00000007
#define HMAC_HASH_CFG_FUNC_MASK         0x00000100
#define HMAC_HASH_PLEN_VALUE_MASK       0xffffffff
#define HMAC_HASH_ALEN_VALUE_MASK       0xffffffff

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_HMAC_REGS_H__*/
