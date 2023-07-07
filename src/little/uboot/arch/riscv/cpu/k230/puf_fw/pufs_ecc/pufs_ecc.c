/**
 * @file      pufs_ecc.c
 * @brief     PUFsecurity ECC API implementation
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
#include "pufs_ecc_internal.h"

/*****************************************************************************
 * Variables
 ****************************************************************************/
/**
 * NIST standard elliptic curves
 */
pufs_ecc_param_st ecc_param[] =
{
    { /* SM2: */
        "\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff", // prime
        "\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xfc", // a
        "\x28\xe9\xfa\x9e\x9d\x9f\x5e\x34\x4d\x5a\x9e\x4b\xcf\x65\x09\xa7\xf3\x97\x89\xf5\x15\xab\x8f\x92\xdd\xbc\xbd\x41\x4d\x94\x0e\x93", // b
        "\x32\xc4\xae\x2c\x1f\x19\x81\x19\x5f\x99\x04\x46\x6a\x39\xc9\x94\x8f\xe3\x0b\xbf\xf2\x66\x0b\xe1\x71\x5a\x45\x89\x33\x4c\x74\xc7", // px
        "\xbc\x37\x36\xa2\xf4\xf6\x77\x9c\x59\xbd\xce\xe3\x6b\x69\x21\x53\xd0\xa9\x87\x7c\xc6\x2a\x47\x40\x02\xdf\x32\xe5\x21\x39\xf0\xa0", // py
        "\xff\xff\xff\xfe\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x72\x03\xdf\x6b\x21\xc6\x05\x2b\x53\xbb\xf4\x09\x39\xd5\x41\x23", // order
        256, 256, 2/* XXX field type */, 1, 32, true,
    },
};

/*****************************************************************************
 * Internal functions
 ****************************************************************************/
/**
 * reverse()
 */
void reverse(uint8_t* dst, const uint8_t* src, size_t len)
{
    for (size_t i=0,j=len-1; i<len; i++,j--)
        dst[i] = src[j];
}

func_reverse cb_reverse = reverse;