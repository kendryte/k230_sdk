/**
 * @file      pufs_ka.c
 * @brief     PUFsecurity KA API implementation
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
#include "pufs_ka_internal.h"
#include <common.h>
#include "platform.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")
struct pufs_ka_regs  *ka_regs   = (struct pufs_ka_regs *)(PUFIOT_ADDR_START+KA_ADDR_OFFSET);
struct pufs_kwp_regs *kwp_regs = (struct pufs_kwp_regs *)(PUFIOT_ADDR_START+KWP_ADDR_OFFSET);

/*****************************************************************************
 * Macros
 ****************************************************************************/
#define IV_BLOCK_SIZE 16
#define SK_SIZE 4
#define PK_SIZE 4
#define SS_SIZE 4

/**
 * @brief Starting KWP and wait until done
 *
 * @return KWP execution status
 */
static pufs_status_t pufs_kwp_start(void)
{
    uint32_t val32;
    kwp_regs->start = 0x1;
    
    while(((val32 = kwp_regs->status) & KWP_STATUS_BUSY_MASK) != 0);

    if (val32 & (0x1 << 2))
        return E_DENY;
    else if (val32 & (0x1 << 3))
        return E_OVERFLOW;
    else if (val32 & (0x1 << 4))
        return E_UNDERFLOW;
    else if (val32 & (0x1 << 5))
        return E_VERFAIL;
    else if (val32 != 0)
    {
        LOG_ERROR("KWP status: 0x%08"PRIx32"\n", val32);
        return E_ERROR;
    }

    return SUCCESS;
}

/**
 * @brief check session/secret key slot in KeyArray by key length and registers
 *
 * @param[in] valid    Check the KA register to ensure the key is valid if true.
 * @param[in] slot     The session/secret key slot which the key is stored in.
 * @param[in] keybits  The key length in bits.
 * @return             SUCCESS on success, otherwise an error code.
 */
static pufs_status_t ka_skslot_check(bool valid, pufs_ka_slot_t slot, uint32_t keybits)
{
    // check keybits
    switch (slot)
    {
    case SK128_0:
    case SK128_1:
    case SK128_2:
    case SK128_3:
    case SK128_4:
    case SK128_5:
    case SK128_6:
    case SK128_7:
        if (keybits > 128)
            return E_OVERFLOW;
        break;
    case SK256_0:
    case SK256_1:
    case SK256_2:
    case SK256_3:
        if (keybits > 256)
            return E_OVERFLOW;
        else if ((keybits <= 128) && (keybits != 0))
            return E_UNDERFLOW;
        break;
    case SK512_0:
    case SK512_1:
        if (keybits > 512)
            return E_OVERFLOW;
        else if ((keybits <= 256) && (keybits != 0))
            return E_UNDERFLOW;
        break;
    default:
        return E_INVALID;
    }

    // check registers
    if (valid)
    {
        uint32_t key_info;
        uint32_t idx;
        uint32_t tagbase;

        switch (slot)
        {
        case SK128_0:
        case SK128_1:
        case SK128_2:
        case SK128_3:
        case SK128_4:
        case SK128_5:
        case SK128_6:
        case SK128_7:
            idx = slot - SK128_0;
            tagbase = 0x30;
            key_info = ka_regs->sk[idx];
            if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
                (((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
                (((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)))
                return E_INVALID;
            break;
        case SK256_0:
        case SK256_1:
        case SK256_2:
        case SK256_3:
            idx = slot - SK256_0;
            tagbase = 0x50;
            key_info = ka_regs->sk[2 * idx];
            if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
                (((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
                (((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)) ||
                (ka_regs->sk[2*idx+1] != ((tagbase + idx) << 16)))
                return E_INVALID;
            break;
        case SK512_0:
        case SK512_1:
            idx = slot - SK512_0;
            tagbase = 0x60;
            key_info = ka_regs->sk[4 * idx];
            if (((key_info & SK_KEY_VAILD_MASK) == 0) ||
                (((key_info & SK_KEY_SIZE_MASK) >> 4) != keybits) ||
                (((key_info & SK_KEY_TAG_MASK) >> 16) != (tagbase + idx)) ||
                (ka_regs->sk[4*idx+1] != ((tagbase + idx) << 16)) ||
                (ka_regs->sk[4*idx+2] != ((tagbase + idx) << 16)) ||
                (ka_regs->sk[4*idx+3] != ((tagbase + idx) << 16)))
                return E_INVALID;
            break;
        default:
            return E_INVALID;
        }
    }

    return SUCCESS;
}

/**
 * keyslot_check()
 */
pufs_status_t keyslot_check(bool valid, pufs_key_type_t keytype, uint32_t slot, uint32_t keybits)
{
    switch (keytype)
    {
    case SSKEY:
        return ka_skslot_check(valid, (pufs_ka_slot_t)slot, keybits);
    default:
        return E_INVALID;
    }
}

/**
 * get_key_slot_idx()
 */
int get_key_slot_idx(pufs_key_type_t keytype, uint32_t keyslot)
{
    switch (keytype)
    {
    case SWKEY:
        return 0;
    case OTPKEY:
        return (keyslot - OTPKEY_0);
    case SSKEY:
        switch ((pufs_ka_slot_t)keyslot)
        {
        case SK128_0:
        case SK128_1:
        case SK128_2:
        case SK128_3:
        case SK128_4:
        case SK128_5:
        case SK128_6:
        case SK128_7:
            return (keyslot - SK128_0);
        case SK256_0:
        case SK256_1:
        case SK256_2:
        case SK256_3:
            return ((keyslot - SK256_0) * 2);
        case SK512_0:
        case SK512_1:
            return ((keyslot - SK512_0) * 4);
        default:
            return -1;
        }
        break;
    default:
        return -1;
    }
}

/**
 * pufs_import_plaintext_key()
 */
pufs_status_t pufs_import_plaintext_key(pufs_key_type_t keytype, pufs_ka_slot_t slot, const uint8_t* key, uint32_t keybits)
{
    pufs_status_t check;
    // keytype MUST be either SSKEY or PRKEY
    if ((keytype != SSKEY) && (keytype != PRKEY))
        return E_INVALID;
    // check KA key slot by key length
    if ((check = keyslot_check(false, keytype, slot, keybits)) != SUCCESS)
        return check;

    // Register manipulation
    if (kwp_regs->status & KWP_STATUS_BUSY_MASK)
        return E_BUSY;

    uint32_t val32;

    val32 = 0x0<<0 | keybits<<8;
    switch (keytype)
    {
    case SSKEY:
        val32 |= 0x0<<19;
        break;
    case PRKEY:
        val32 |= 0x1<<19;
        break;
    default:
        return E_FIRMWARE;
    }
    val32 |= get_key_slot_idx(keytype, slot)<<20;
    kwp_regs->cfg = val32;

    memset(pufs_buffer, 0, KWP_KEY_MAXLEN);
    memcpy(pufs_buffer, key, b2B(keybits));

    uint32_t *buf = (uint32_t *)pufs_buffer;
    for (int i = 0; i < KWP_KEY_MAXLEN / 4; ++i)
        kwp_regs->key[i] = cb_be2le(buf[i]);

    return pufs_kwp_start();
}

func_get_key_slot_idx cb_get_key_slot_idx = get_key_slot_idx;
#pragma GCC pop_options