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
// struct pufs_kwp_regs *kwp_regs = (struct pufs_kwp_regs *)(PUFIOT_ADDR_START+KWP_ADDR_OFFSET);

/*****************************************************************************
 * Macros
 ****************************************************************************/
#define IV_BLOCK_SIZE 16
#define SK_SIZE 4
#define PK_SIZE 4
#define SS_SIZE 4

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
    default:
        return -1;
    }
}

func_get_key_slot_idx cb_get_key_slot_idx = get_key_slot_idx;
#pragma GCC pop_options