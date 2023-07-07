/**
 * @file      pufs_dma.c
 * @brief     PUFsecurity DMA API implementation
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
#include "pufs_dma_internal.h"
#include "pufs_rt_internal.h"
#include "platform.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")

struct pufs_dma pufs_dma = {.regs = (struct pufs_dma_regs *)(PUFIOT_ADDR_START+DMA_ADDR_OFFSET)};

/*****************************************************************************
 * Internal functions
 ****************************************************************************/
void dma_write_rwcfg(const uint8_t *out, const uint8_t *in, uint32_t len)
{
    pufs_dma.regs->dsc_cfg_2 = len;

    pufs_dma.regs->dsc_cfg_0 = (uintptr_t)in;
    pufs_dma.regs->dsc_cfg_1 = (uintptr_t)out;
}

void dma_write_key_config_0(pufs_key_type_t keytype, pufs_algo_type_t algo, uint32_t size, uint32_t slot_index)
{
    uint32_t value = 0;
    value |= slot_index << DMA_KEY_CFG_0_KEY_IDX_BITS;
    value |= size << DMA_KEY_CFG_0_KEY_SIZE_BITS;
    value |= algo << DMA_KEY_CFG_0_KEY_DST_BITS;
    value |= keytype;
    pufs_dma.regs->key_cfg_0 = value;
}

void dma_write_config_0(bool rng_enable, bool sgdma_enable, bool no_cypt)
{
    pufs_dma.regs->cfg_0 = 0;
}

void dma_write_cl_config_0(uint32_t value)
{
    pufs_dma.regs->cl_cfg_0 = value;
}

void dma_write_data_block_config(bool head, bool tail, bool dn_intrpt, bool dn_pause, uint32_t offset)
{
    uint32_t value = 0;
    if (head)
        value |= 0x1 << DMA_DSC_CFG_4_HEAD_BITS;
    if (tail)
        value |= 0x1 << DMA_DSC_CFG_4_TAIL_BITS;
    if (dn_intrpt)
        value |= 0x1 << DMA_DSC_CFG_4_DN_INTRPT_BITS;
    if (dn_pause)
        value |= 0x1 << DMA_DSC_CFG_4_DN_PAUSE_BITS;
    value |= offset << DMA_DSC_CFG_4_OFFSET_BITS;
    
    pufs_dma.regs->dsc_cfg_4 = value;
}

void dma_write_start(void)
{
    pufs_dma.regs->start = 0x1;
}

bool dma_check_busy_status(uint32_t *status)
{
    uint32_t stat = pufs_dma.regs->status_0;
    bool busy = (stat & DMA_STATUS_0_BUSY_MASK) != 0;
    
    if (status != NULL)
        *status = stat;

    return busy;
}

func_dma_write_rwcfg cb_dma_write_rwcfg = dma_write_rwcfg;
func_dma_write_key_config_0 cb_dma_write_key_config_0 = dma_write_key_config_0;
func_dma_write_config_0 cb_dma_write_config_0 = dma_write_config_0;
func_dma_write_cl_config_0 cb_dma_write_cl_config_0 = dma_write_cl_config_0;
func_dma_write_data_block_config cb_dma_write_data_block_config = dma_write_data_block_config;
func_dma_write_start cb_dma_write_start = dma_write_start;
func_dma_check_busy_status cb_dma_check_busy_status = dma_check_busy_status;
#pragma GCC pop_options