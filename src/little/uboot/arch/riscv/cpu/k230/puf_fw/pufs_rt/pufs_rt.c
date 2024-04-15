/**
 * @file      pufs_rt.c
 * @brief     PUFsecurity PUFrt API implementation
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
#include "pufs_internal.h"
#include "pufs_rt_internal.h"
#include "platform.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")

struct pufs_rt_regs *rt_regs = (struct pufs_rt_regs *)(PUFIOT_ADDR_START+RT_ADDR_OFFSET);

/**
 * pufs_read_otp()
 */
pufs_status_t pufs_read_otp(uint8_t* outbuf, uint32_t len, pufs_otp_addr_t addr)
{
    uint32_t start_index, wlen;

    wlen = len / WORD_SIZE;
    start_index = addr / WORD_SIZE;

    if (wlen > 0)
    {
        memcpy(outbuf, (void *)(rt_regs->otp + start_index), wlen * WORD_SIZE);

        uint32_t *out32 = (uint32_t *)outbuf;
        for (size_t i = 0; i < wlen; ++i)
            *(out32 + i) = cb_be2le(*(out32 + i));
    }

    return SUCCESS;
}

#ifndef BOOTROM
/*****************************************************************************
 * Static functions
 ****************************************************************************/
/**
 * @brief Select the OTP R/W lock register corresponding to the OTP slot index
 */
static size_t rwlck_index_sel(uint32_t idx)
{
    uint32_t group = idx / RWLOCK_GROUP_OTP;

    if (group >= MAX_RWLOCK_GROUPS)
        return -1;
    
    return PIF_RWLCK_START_INDEX + group; 
}

static bool check_enable(uint32_t value)
{
    value &= 0xF;
    switch (value)
    {
    case PUFRT_VALUE4(0x0):
    case PUFRT_VALUE4(0x1):
    case PUFRT_VALUE4(0x2):
    case PUFRT_VALUE4(0x4):
        return true;
    default:
        return false;
    }
}

/*****************************************************************************
 * Internal functions
 ****************************************************************************/

/**
 * @brief validate OTP key slot by key length
 *
 * @param slot The OTP key slot which the key is stored in.
 * @param keybits The key length in bits.
 * @return SUCCESS on success, otherwise an error code.
 */
pufs_status_t otpkey_slot_check(pufs_rt_slot_t slot, uint32_t keybits)
{
    // check slot is in OTPKEY_[0-31], and of size less than 2048
    if ((slot < OTPKEY_0) || (slot > OTPKEY_31))
        return E_INVALID;
    if (keybits > 2047)
        return E_OVERFLOW;
    // produce the alignment factor
    uint32_t align = 1;
    for (uint32_t slotbits = OTP_KEY_BITS; keybits > slotbits; slotbits*=2, align*=2);
    // key slot alignment check
    if (((slot - OTPKEY_0) % align) != 0)
        return E_ALIGN;

    return SUCCESS;
}
/**
 * @brief validate OTP access range test
 *
 * @param addr    The OTP address which the data is stored in.
 * @param keybits The length in bytes.
 * @return SUCCESS on success, otherwise an error code.
 */
static pufs_status_t otp_range_check(pufs_otp_addr_t addr, uint32_t len)
{
    // word-aligned OTP address check
    if ((addr % WORD_SIZE) != 0)
        return E_ALIGN;
    // OTP boundary check
    if ((len > OTP_LEN) || (addr > (OTP_LEN - len)))
        return E_OVERFLOW;
    return SUCCESS;
}
/**
 * @brief wait PUFrt busy status
 *
 * @return  PTM status
 */
uint32_t wait_status(void)
{
    while((rt_regs->status & PTM_STATUS_BUSY_MASK) != 0);
    return rt_regs->status;
}
#define pufs_ptm_cfg_set(mask, ...) \
    _pufs_ptm_cfg_set(mask, DEF_ARG(__VA_ARGS__, true))
/**
 * @brief Set/unset PTM cfg bit
 *
 * @param[in] mask  The bit.
 * @param[in] on    Turn on/off the bit according to true/false.
 */
static void _pufs_ptm_cfg_set(uint32_t mask, bool on, bool wait)
{
    if (on)
        rt_regs->cfg |= mask; 
    else
        rt_regs->cfg = rt_regs->cfg & (~mask); 
    
    if (wait)
        wait_status();
}
/**
 * @brief Continuous random output control
 *
 * @param[in] on  Turn on/off continuous random output when true/false is
 *                specified.
 */
void pufs_rng_cont_ctrl(bool on)
{
    pufs_ptm_cfg_set(PTM_CFG_REG_RNG_CONT_MASK, on);
}
/**
 * @brief Continuous entropy output control
 *
 * @param[in] on  Turn on/off continuous entropy output when true/false is
 *                specified.
 */
void pufs_fre_cont_ctrl(bool on)
{
    pufs_ptm_cfg_set(PTM_CFG_REG_FRE_COUNT_MASK, on);
}
/**
 * @brief PTR/PTC control
 *
 * @param[in] on  Turn on/off PTR/PTC access when true/false is specified.
 */
void pufs_ptr_ptc_ctrl(bool on)
{
    pufs_ptm_cfg_set(PTM_CFG_REG_PTR_PTC_MASK, on);
}
/**
 * @brief Deep standby control (active low)
 *
 * @param[in] on  Turn off/on deep standby mode when true/false is specified.
 */
void rt_write_pdstb(bool off)
{
    pufs_ptm_cfg_set(PTM_CFG_REG_PDSTB_MASK, off, (off ? true : false));
}

// Program protect为了保证已经烧录为1的cell不会再烧一次。
// Program ignore为了保证跳过为0的cell，减少电路stress。
// 不知道为什么ip厂商不把这两个功能配置为默认使能
// 在进行编程之前务必确认已启用program protect (done at factory) 以及启用program ignore(enable config reg)
void puf_pgm_ign_ctrl(bool on)
{
    pufs_ptm_cfg_set(PTM_CFG_REG_PGM_IGN_MASK, on);
}
//puf_pgm_ign_ctrl(true) to enable PGM_IGN
/**
 * @brief Read mode control
 *
 * @param[in] mode  Read mode
 */
void rt_write_read_mode(pufs_read_mode_t mode)
{
    rt_regs->cfg = (rt_regs->cfg & ~(0x3 << 4))| mode;
}
/**
 * @brief Get OTP rwlck value
 *
 * @param[in] addr  The address of the rwlck.
 * @return          The rwlck bits.
 */
pufs_otp_lock_t pufs_get_otp_rwlck(pufs_otp_addr_t addr)
{
    pufs_status_t check;

    if ((check = otp_range_check(addr, 4)) != SUCCESS)
        return check;

    // get rwlck
    int idx = addr / WORD_SIZE;
    int group_index = idx % RWLOCK_GROUP_OTP;

    if ((idx = rwlck_index_sel(idx)) == -1)
        return E_INVALID;

    uint32_t lck = (rt_regs->pif[idx] >> (group_index * WORD_SIZE)) & 0xF;

    switch (lck)
    {
    case PUFRT_VALUE4(0x0):
        return NA;
    case PUFRT_VALUE4(0xC):
        return RO;
    case PUFRT_VALUE4(0xF):
        return RW;
    default:
        return N_OTP_LOCK_T;
    }
}

void rt_write_enroll(void)
{
    if (rt_check_enrolled())
        return;
    rt_regs->puf_enroll = 0xa7;
    wait_status();
}

bool rt_check_rngclk_enable(void)
{
    return ((rt_regs->cfg >> 1) & 0x1) == 1;
}

void rt_write_rngclk(bool enable)
{
    if (enable)
        rt_regs->cfg |= 0x2;
    else
        rt_regs->cfg &= ~0x2;
}

/**
 * _pufs_get_uid()
 */
pufs_status_t _pufs_get_uid(pufs_uid_st* uid, pufs_rt_slot_t slot)
{
    uint32_t index, *uid32;
    switch (slot)
    {
    case PUFSLOT_0:
        index = 0;
        break;
    case PUFSLOT_1:
        index = 8;
        break;
    case PUFSLOT_2:
        index = 16;
        break;
    case PUFSLOT_3:
        index = 24;
        break;
    default:
        return E_INVALID;
    }

    memcpy(uid->uid, (void *)(rt_regs->puf + index), UIDLEN);

    uid32 = (uint32_t *)uid->uid;
    for (size_t i = 0; i < (UIDLEN / 4); ++i)
        *(uid32 + i) = cb_be2le(*(uid32 + i));

    return SUCCESS;
}
/**
 * _pufs_rand()
 */
pufs_status_t _pufs_rand(uint8_t* rand, uint32_t numblks)
{
    uint32_t* crand = (uint32_t*)rand;

    // Register manipulation
    for (uint32_t i=0;i<numblks;i++)
        crand[i] = rt_regs->rn;

    return SUCCESS;
}

/**
 * pufs_program_otp()
 */
pufs_status_t pufs_program_otp(const uint8_t* inbuf, uint32_t len,
                               pufs_otp_addr_t addr)
{
    pufs_status_t check;
    uint32_t start_index = addr / WORD_SIZE;

    if ((check = otp_range_check(addr, len)) != SUCCESS)
        return check;
    puf_pgm_ign_ctrl(true);
    // program
    for (uint32_t i=0;i<len;i+=4)
    {
        union {
            uint32_t word;
            uint8_t byte[4];
        } otp_word;
        for (int8_t j=3;j>=0;j--) // reserve, default 0xff
            otp_word.byte[j] = ((i+3-j) < len) ? inbuf[i+3-j] : 0xff;
            
        if(otp_word.word == 0x0) continue;
        if(rt_regs->otp[start_index + (i/4)] == otp_word.word) continue;
        
        // printf("[%d]:0x%x \n", start_index + (i/4), otp_word.word);
        rt_regs->otp[start_index + (i/4)] = otp_word.word;
    }

    return SUCCESS;
}
/**
 * pufs_lock_otp
 */
pufs_status_t pufs_lock_otp(pufs_otp_addr_t addr, uint32_t len,
                            pufs_otp_lock_t lock)
{
    pufs_status_t check;
    uint32_t lock_val = 0, shift = 0, start = 0, end = 0, mask = 0, val32 = 0, rwlock_index;

    if ((check = otp_range_check(addr, len)) != SUCCESS)
        return check;

    switch (lock)
    {
    case NA:
        lock_val = PUFRT_VALUE4(0x0);
        break;
    case RO:
        lock_val = PUFRT_VALUE4(0xC);
        break;
    case RW:
        lock_val = PUFRT_VALUE4(0xF);
        break;
    default:
        return E_INVALID;
    }

    end = (len + 3) / 4;
    start = addr / WORD_SIZE;

    for (uint32_t i = 0; i < end; i++)
    {
        int idx = start + i;
        rwlock_index = rwlck_index_sel(idx);

        shift = (idx % RWLOCK_GROUP_OTP) * 4;
        val32 |= lock_val << shift;
        mask |= 0xF << shift;

        if (shift == 28 || i == end - 1)
        {
            val32 |= (rt_regs->pif[rwlock_index] & (~mask));
            rt_regs->pif[rwlock_index] = val32;

            val32 = 0;
            mask = 0;
        }
    }

    return SUCCESS;
}
/**
 * pufs_program_key2otp
 */
pufs_status_t pufs_program_key2otp(pufs_rt_slot_t slot, const uint8_t* key,
                                   uint32_t keybits)
{
    pufs_status_t check;
    // check OTP key slot by key length
    if ((check = otpkey_slot_check(slot, keybits)) != SUCCESS)
        return check;

    // XXX programmed check
    // write key by pufs_program_otp()
    pufs_otp_addr_t addr = (slot - OTPKEY_0) * OTP_KEY_LEN;
    if ((check = pufs_program_otp(key, b2B(keybits), addr)) != SUCCESS)
        return check;

    // set rwlck to 3'b000
    return pufs_lock_otp(addr, b2B(keybits), NA);
}
/**
 * pufs_zeroize()
 */
pufs_status_t pufs_zeroize(pufs_rt_slot_t slot)
{
    uint32_t val32;

    if (slot == PUFSLOT_0) {
        val32 = 0x4b;
    } else if (slot == PUFSLOT_1) {
        val32 = 0xad;
    } else if (slot == PUFSLOT_2) {
        val32 = 0xd2;
    } else if (slot == PUFSLOT_3) {
        val32 = 0x34;
    } else if ((slot >= OTPKEY_0) && (slot <= OTPKEY_31)) {
        val32 = (slot - OTPKEY_0) + 0x80;
    } else {
        return E_INVALID;
    }
    
    if (slot < OTPKEY_0)
        rt_regs->puf_zeroize = val32;
    else
        rt_regs->otp_zeroize = val32;


    val32 = wait_status();

    if ((val32 & 0x0000001e) != 0)
    {
        LOG_ERROR("PUFRT status: 0x%x\n", val32);
        return E_ERROR;
    }
    return SUCCESS;
}
/**
 * pufs_post_mask()
 */
pufs_status_t pufs_post_mask(uint64_t maskslots)
{
    uint32_t otp_psmsk_0, otp_psmsk_1, puf_psmsk, val32_0 = 0, val32_1 = 0;

    puf_psmsk = (maskslots & 0xf);
    otp_psmsk_0 = ((maskslots>>4) & 0x0000ffff);
    otp_psmsk_1 = ((maskslots>>20) & 0x0000ffff);

    for (int i=0; i<16; i++)
    {
        if ((otp_psmsk_0 >> i) & 0x1)
            val32_0 |= (0x3 << (2*i));
        if ((otp_psmsk_1 >> i) & 0x1)
            val32_1 |= (0x3 << (2*i));
    }

    rt_regs->otp_psmsk[0] = val32_0;
    rt_regs->otp_psmsk[1] = val32_1;

    // also enable post-masking
    val32_0 = PMK_LCK_PSMSK_MASK;
    for (int i=0; i<4; i++)
    {
        if ((puf_psmsk >> i) & 0x1)
            val32_0 |= (0x3 << (2*i));
    }

    rt_regs->puf_psmsk = val32_0;

    return SUCCESS;
}
/**
 * pufs_rt_version()
 */
pufs_status_t pufs_rt_version(uint32_t* version, uint32_t* features)
{
    *version = rt_regs->version;
    *features = rt_regs->feature;
    return SUCCESS;
}

bool rt_check_enrolled(void)
{
    return check_enable(rt_regs->pif[0] >> PIF_00_ENROLL_BITS);
}

bool rt_check_tmlck_enable(void)
{
    return check_enable(rt_regs->pif[0] >> PIF_00_TMLCK_BITS);
}

bool rt_check_puflck_enable(void)
{
    return check_enable(rt_regs->pif[0] >> PIF_00_PUFLCK_BITS);
}

bool rt_check_otplck_enable(void)
{
    return check_enable(rt_regs->pif[0] >> PIF_00_OTPLCK_BITS);
}

bool rt_check_shfwen_enable(void)
{
    return check_enable(rt_regs->pif[0] >> PIF_00_SHFWEN_BITS);
}

bool rt_check_shfren_enable(void)
{
    return check_enable(rt_regs->pif[0] >> PIF_00_SHFREN_BITS);
}

bool rt_check_pgmprt_enable(void)
{
    return check_enable(rt_regs->pif[0] >> PIF_00_PGMPRT_BITS);
}

pufs_status_t rt_write_set_flag(pufs_ptm_flag_t flag, uint32_t *status)
{
    uint32_t res;
    if (rt_check_tmlck_enable())
        return E_INVALID;

    rt_regs->set_flag = flag;
    res = wait_status();
    if (status != NULL)
        *status = res;

    return SUCCESS;
}
#endif

func_pufs_read_otp cb_pufs_read_otp = pufs_read_otp;
//rt--register table
#pragma GCC pop_options