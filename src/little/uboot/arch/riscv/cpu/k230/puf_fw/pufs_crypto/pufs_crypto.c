/**
 * @file      pufs_crypto.c
 * @brief     PUFsecurity Crypto Engine API
 * @copyright 2021 PUFsecurity
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

#include "pufs_reg_ctrl.h"
#include "pufs_internal.h"
#include "pufs_dma_internal.h"
#include "pufs_crypto_internal.h"
#include "platform.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")
volatile struct pufs_crypto_regs *crypto_regs = (struct pufs_crypto_regs *)(PUFIOT_ADDR_START+CRYPTO_ADDR_OFFSET);

/*****************************************************************************
 * Static functions
 ****************************************************************************/

// note: due to the 32bit bus, the length of src MUST be a multiple of 4
static void write_data(uint32_t *dst, uint8_t *src, size_t length, bool le)
{
    uint32_t *src32 = (uint32_t *)src;
    length = length / 4;

    for (size_t i = 0; i < length; ++i)
    {
        *(dst + i) = cb_be2le(*(src32 + i));
    }
}

// Assume it is always valid to read (length / byte_per_word) + 1 words from src address.
static void read_data(uint8_t *dst, uint32_t *src, size_t length, bool le)
{
    uint32_t *dst32 = (uint32_t *)dst;
    uint32_t word_nums = 0;
    word_nums = length / 4;

    for (uint32_t i = 0; i < word_nums; i++)
    {
        dst32[i] = cb_be2le(src[i]);
    }
}

void _crypto_write_regs(uint32_t offset, uint32_t index, uint32_t value)
{
    *((uint32_t *)(((uintptr_t)crypto_regs) + offset + index)) = value;
}

pufs_status_t crypto_write_sw_key(uint8_t *key, size_t length)
{
    cb_write_data((uint32_t *)crypto_regs->sw_key, key, length, true);
    return SUCCESS;
}

pufs_status_t crypto_write_iv(uint8_t *iv, size_t length)
{
    cb_write_data((uint32_t *)crypto_regs->iv, iv, length, true);
    return SUCCESS;
}

pufs_status_t crypto_read_iv(uint8_t *out, size_t length)
{
    cb_read_data(out, (void *)crypto_regs->iv_out, length, true);
    return SUCCESS;
}

pufs_status_t crypto_write_dgst(uint8_t *dgst, size_t length)
{
    cb_write_data((uint32_t *)crypto_regs->dgst_in, dgst, length, true);
    return SUCCESS;
}

// note: due to the 32bit bus, the length of src MUST be a multiple of 4
void crypto_read_dgest(uint8_t *out, size_t length)
{
    cb_read_data(out, (void *)crypto_regs->dgst_out, length, true);
}

func_write_data cb_write_data = write_data;
func_read_data cb_read_data = read_data;
func__crypto_write_regs cb__crypto_write_regs = _crypto_write_regs;
func_crypto_write_sw_key cb_crypto_write_sw_key = crypto_write_sw_key;
func_crypto_write_iv cb_crypto_write_iv = crypto_write_iv;
func_crypto_read_iv cb_crypto_read_iv = crypto_read_iv;
func_crypto_write_dgst cb_crypto_write_dgst = crypto_write_dgst;
func_crypto_read_dgest cb_crypto_read_dgest = crypto_read_dgest;

#pragma GCC pop_options