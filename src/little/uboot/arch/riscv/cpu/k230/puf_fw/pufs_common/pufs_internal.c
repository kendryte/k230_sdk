/**
 * @file      pufs_internal.c
 * @brief     PUFsecurity common API implementation
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

#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif /* __USE_POSIX199309 */
#include "pufs_internal.h"

/*****************************************************************************
 * Variables
 ****************************************************************************/
uint8_t pufs_buffer[BUFFER_SIZE];

/*****************************************************************************
 * Hardlock
 ****************************************************************************/
void hardlock_config(pufs_lock_t lock_cfg)
{
    void *lock_base_addr = 0x911040b8;

    switch(lock_cfg)
    {
        case lock:
        {
            while((*(uint32_t *)lock_base_addr) == 1);
            *(uint32_t *)lock_base_addr = 0x1;

            break;
        }

        case unlock:
        {
            if(*(uint32_t *)lock_base_addr)
                *(uint32_t *)lock_base_addr = 0x0;

            break;
        }

        default:
            break;
    }
}

/*****************************************************************************
 * Internal functions
 ****************************************************************************/
/**
 * segment()
 */
blsegs segment(uint8_t * buf, uint32_t buflen, const uint8_t* in,
               uint32_t inlen, uint32_t blocksize, uint32_t minlen)
{
    blsegs ret = { .nsegs = 0 };

    // calculate total number of blocks to be processed
    uint32_t nprocblocks = 0;
    if ((buflen + inlen) >= (minlen + blocksize))
        nprocblocks = (buflen + inlen - minlen) / blocksize;

    // no available block for processing, keep input in the internal buffer.
    if (nprocblocks == 0)
    {
        ret.seg[ret.nsegs++] = (segstr){ false, buf, buflen };
        ret.seg[ret.nsegs++] = (segstr){ false, in, inlen };
        return ret;
    }

    const uint8_t* start = in;
    // some blocks are ready for processing,
    // using bytes in the internal buffer first
    if (buflen != 0)
    {
        // if all data in the internal buffer will be processed
        if (nprocblocks * blocksize >= buflen)
        {
            // fill buffer if not a complete block
            uint32_t proclen = blocksize;
            nprocblocks--;
            while (proclen < buflen)
            {
                proclen += blocksize;
                nprocblocks--;
            }
            memcpy(buf + buflen, start, proclen - buflen);
            ret.seg[ret.nsegs++] = (segstr){ true, buf, proclen };
            start += (proclen - buflen);
            inlen -= (proclen - buflen);
        }
        else // some data will be remained in the internal buffer
        {
            ret.seg[ret.nsegs++] = (segstr){ true, buf,
                                             nprocblocks * blocksize };
            ret.seg[ret.nsegs++] = (segstr){ false,
                                             buf + nprocblocks * blocksize,
                                             buflen - nprocblocks * blocksize };
            nprocblocks = 0;
        }
    }
    // deal with input data
    if (nprocblocks > 0)
    {
        ret.seg[ret.nsegs++] = (segstr){ true, start, nprocblocks * blocksize };
    }
    ret.seg[ret.nsegs++] = (segstr){ false, start + nprocblocks * blocksize,
                                     inlen - nprocblocks * blocksize };
    return ret;
}

func_segment cb_segment = segment;