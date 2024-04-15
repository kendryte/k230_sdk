/**
 * @file      pufs_rt_internal.h
 * @brief     PUFsecurity PUFrt internal interface
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

#ifndef __PUFS_RT_INTERNAL_H__
#define __PUFS_RT_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pufs_rt.h"
#include "pufs_rt_regs.h"

/*****************************************************************************
 * Macros
 ****************************************************************************/

#define PUFSRT_VERSION 0x504D30D6
#define WORD_SIZE 4 // 4 bytes = 32 bits = an otp block
#define RWLOCK_GROUP_OTP 8 
#define MAX_RWLOCK_GROUPS OTP_LEN / WORD_SIZE / RWLOCK_GROUP_OTP  // a group contains 8 otp blocks

/*****************************************************************************
 * Variables
 ****************************************************************************/
struct pufs_rt_cde_regs{
    volatile uint32_t otp[768]; // 24K-bit OTP
};

extern struct pufs_rt_regs *rt_regs;
extern struct pufs_rt_cde_regs *rt_cde_regs;
#define WORD_SIZE 4 // 4 bytes = 32 bits = an otp block
#define RWLOCK_GROUP_OTP 8 
#define MAX_RWLOCK_GROUPS OTP_LEN / WORD_SIZE / RWLOCK_GROUP_OTP  // a group contains 8 otp blocks

/*****************************************************************************
 * Enumerations
 ****************************************************************************/
/**
 * @brief read mode
 */
typedef enum {
    RD,             ///< Read
    OFF_MG_RD,      ///< Off-margin read
    ON_MG_RD,       ///< On-margin read
    EXT_OFF_MG_RD,  ///< Extended off-margin read
    N_READ_MODE_T,
} pufs_read_mode_t;

typedef enum {
    PGMPRT_FLAG = 0xb6,             
    TMLCK_FLAG = 0x71,      
    OTPLCK_FLAG = 0x65,       
    PUFLCK_FLAG = 0x2c,  
    SHFREN_FLAG = 0x99,
    SHFWEN_FLAG = 0xc2,
    SET_SHFREN_FLAG = 0xe3,
    SET_SHFWEN_FLAG = 0x8e,
} pufs_ptm_flag_t;

typedef enum {
    PUFORG_0 = 0x12,
    PUFORG_1 = 0x13,
    PA_REP_0 = 0x38,
    PA_REP_1 = 0x39,
} pufs_ptm_pin_t;

/*****************************************************************************
 * Internal functions
 ****************************************************************************/
/**
 * @brief validate PUF slot
 *
 * @param slot The PUF slot which the key is stored in.
 * @return SUCCESS on success, otherwise an error code.
 */
pufs_status_t puf_slot_check(pufs_rt_slot_t slot);
/**
 * @brief validate OTP key slot by key length
 *
 * @param slot The OTP key slot which the key is stored in.
 * @param keybits The key length in bits.
 * @return SUCCESS on success, otherwise an error code.
 */
pufs_status_t otpkey_slot_check(pufs_rt_slot_t slot, uint32_t keybits);
/**
 * @brief wait PUFrt busy status
 *
 * @return  PTM status
 */
uint32_t wait_status(void);
/**
 * @brief Continuous random output control
 *
 * @param[in] on  Turn on/off continuous random output when true/false is
 *                specified.
 */
void pufs_rng_cont_ctrl(bool on);
/**
 * @brief Continuous entropy output control
 *
 * @param[in] on  Turn on/off continuous random output when true/false is
 *                specified.
 */
void pufs_fre_cont_ctrl(bool on);
/**
 * @brief PTR/PTC control
 *
 * @param[in] on  Turn on/off PTR/PTC access when true/false is specified.
 */
void pufs_ptr_ptc_ctrl(bool on);
/**
 * @brief Deep standby control
 *
 * @param[in] on  Turn on/off deep-standby mode when true/false is specified.
 */
void rt_write_pdstb(bool on);
void puf_pgm_ign_ctrl(bool on);
/**
 * @brief Read mode control
 *
 * @param[in] mode  Read mode
 */
void rt_write_read_mode(pufs_read_mode_t mode);

void rt_write_enroll(void);

void rt_write_rngclk(bool enable);

pufs_status_t rt_write_set_flag(pufs_ptm_flag_t flag, uint32_t *status);

bool rt_check_enrolled(void);
bool rt_check_shfwen_enable(void);
bool rt_check_shfren_enable(void);
bool rt_check_pgmprt_enable(void);
bool rt_check_tmlck_enable(void);
bool rt_check_puflck_enable(void);
bool rt_check_otplck_enable(void);
bool rt_check_rngclk_enable(void);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_RT_INTERNAL_H__ */
