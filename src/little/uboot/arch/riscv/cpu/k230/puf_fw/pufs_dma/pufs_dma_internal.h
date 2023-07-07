/**
 * @file      pufs_dma_internal.h
 * @brief     PUFsecurity DMA internal interface
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

#ifndef __PUFS_DMA_INTERNAL_H__
#define __PUFS_DMA_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "pufs_dma.h"
#include "pufs_ka.h"
#include "pufs_dma_regs.h"
//#include "core_rv64.h"

#define DMA_VERSION 0x505000A1

#define DEFAULT_DESC_MEM_SIZE 2048
#define SGDMA_DESCRIPTOR_SIZE   32

/*****************************************************************************
 * Macros
 ****************************************************************************/
#undef DMADIRECT
#ifdef BAREMETAL
#ifndef DMABUFFER
#define DMADIRECT
#endif /*DMABUFFER */
#endif /* BAREMETAL */

struct pufs_dma
{
    struct pufs_dma_regs *regs;
};

extern struct pufs_dma pufs_dma;

/*****************************************************************************
 * Internal functions
 ****************************************************************************/
#ifndef BAREMETAL
    #define VIRT_ADDR(addr) ((addr - sg_mem.base_addr) + sg_mem.virt_base_addr)
    #define PHY_ADDR(addr) ((addr - sg_mem.virt_base_addr) + sg_mem.base_addr)
#else
     #define VIRT_ADDR(addr) (addr)
     #define PHY_ADDR(addr) (addr)
#endif

#ifndef DMADIRECT
void clear_dma_read(uint32_t len);
void dma_read_output(uint8_t* addr, uint32_t len);
#else
#define clear_dma_read(...)
#define dma_read_output(...)
#endif /* DMADIRECT */

typedef void (* func_dma_write_rwcfg)(const uint8_t *out, const uint8_t *in, uint32_t len);
typedef void (* func_dma_write_key_config_0)(pufs_key_type_t keytype, pufs_algo_type_t algo, uint32_t size, uint32_t slot_index);
typedef void (* func_dma_write_cl_config_0)(uint32_t value);
typedef void (* func_dma_write_config_0)(bool rng_enable, bool sgdma_enable, bool no_cypt);
typedef void (* func_dma_write_data_block_config)(bool head, bool tail, bool dn_intrpt, bool dn_pause, uint32_t offset);
typedef void (* func_dma_write_start)(void);
typedef bool (* func_dma_check_busy_status)(uint32_t *status);

extern func_dma_write_rwcfg cb_dma_write_rwcfg;
extern func_dma_write_key_config_0 cb_dma_write_key_config_0;
extern func_dma_write_cl_config_0 cb_dma_write_cl_config_0;
extern func_dma_write_config_0 cb_dma_write_config_0;
extern func_dma_write_data_block_config cb_dma_write_data_block_config;
extern func_dma_write_start cb_dma_write_start;
extern func_dma_check_busy_status cb_dma_check_busy_status;

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_DMA_INTERNAL_H__*/
