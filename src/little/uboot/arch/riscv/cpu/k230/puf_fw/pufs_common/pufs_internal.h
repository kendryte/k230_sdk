/**
 * @file      pufs_internal.h
 * @brief     PUFsecurity common internal interface
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

#ifndef __PUFS_INTERNAL_H__
#define __PUFS_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pufs_common.h"
#include "pufs_bare.h"
#include "pufs_reg_ctrl.h"
#include "pufs_log.h"

/*****************************************************************************
 * Macros
 ****************************************************************************/
#define UNUSED(x) (void)(x)
/*****************************************************************************
 * Structures
 ****************************************************************************/
/**
 * structure for segment() return value
 */
typedef struct {
    bool process;
    const uint8_t* addr;
    uint32_t len;
} segstr;
typedef struct {
    uint32_t nsegs;
    segstr seg[3];
} blsegs;

/*****************************************************************************
 * Variables
 ****************************************************************************/
extern bool debug_regs;
extern int memfd;
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 512 // To ensure PKC works, the minimum buffer size is 512
#endif
extern uint8_t pufs_buffer[BUFFER_SIZE];

/*****************************************************************************
 * Hardlock
 ****************************************************************************/
typedef enum
{
    lock = 1,
    unlock,
    NONE,
} pufs_lock_t;
void hardlock_config(pufs_lock_t lock_cfg);

/*****************************************************************************
 * Internal functions
 ****************************************************************************/
/**
 * @brief Buffer manipulation for blockwise operations
 *
 * @param buf The pointer to the internal buffer.
 * @param buflen The length of data in the internal buffer in bytes.
 * @param in The pointer to the input data.
 * @param inlen The length of input data in bytes.
 * @param blocksize The blocksize in bytes for HW processing.
 * @param minlen The minimum length should be kept in internal buffer.
 * @return segments for blockwise operation or buffering data
 */

typedef blsegs (* func_segment)(uint8_t* buf, uint32_t buflen, const uint8_t* in, uint32_t inlen,
               uint32_t blocksize, uint32_t minlen);

extern func_segment cb_segment;
/**
 * @brief Profiling a function of prototype void(void)
 *
 * @param f The function to be profiled
 */

#ifndef BAREMETAL
void profiling(void (*f)(void));
#else
#define profiling(...)
#endif /* BAREMETAL */

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_INTERNAL_H__*/
