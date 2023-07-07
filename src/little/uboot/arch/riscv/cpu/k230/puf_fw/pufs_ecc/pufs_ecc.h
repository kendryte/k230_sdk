/**
 * @file      pufs_ecc.h
 * @brief     PUFsecurity ECC API interface
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

#ifndef __PUFS_ECC_H__
#define __PUFS_ECC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <linux/types.h>

/*****************************************************************************
 * Enumerations
 ****************************************************************************/
/**
 * @brief RSA variant.
 */
typedef enum {
    RSA1024,      ///< RSA-1024
    RSA2048,      ///< RSA-2048
    RSA3072,      ///< RSA-3072
    RSA4096,      ///< RSA-4096
    N_RSA_TYPE_T, // keep in the last one
} pufs_rsa_type_t;
/**
 * @brief NIST standardized elliptic curves.
 */
typedef enum {
    // NISTB163,   ///< NIST B-163
    // NISTB233,   ///< NIST B-233
    // NISTB283,   ///< NIST B-283
    // NISTB409,   ///< NIST B-409
    // NISTB571,   ///< NIST B-571
    // NISTK163,   ///< NIST K-163
    // NISTK233,   ///< NIST K-233
    // NISTK283,   ///< NIST K-283
    // NISTK409,   ///< NIST K-409
    // NISTK571,   ///< NIST K-571
    // NISTP192,   ///< NIST P-192
    // NISTP224,   ///< NIST P-224
    // NISTP256,   ///< NIST P-256
    // NISTP384,   ///< NIST P-384
    // NISTP521,   ///< NIST P-521
    SM2,        ///< SM2
    N_ECNAME_T, // keep in the last one
} pufs_ec_name_t;

/*****************************************************************************
 * Structures
 ****************************************************************************/
/**
 * @brief Elliptic curve (EC) domain parameters
 */
typedef struct
{
    const void* field; ///< Field modulus.
    const void* a;     ///< EC parameter a.
    const void* b;     ///< EC parameter b.
    const void* px;    ///< x-coordinate of base point P.
    const void* py;    ///< y-coordinate of base point P.
    const void* order; ///< Subgroup order.
    uint16_t fbits;    ///< Field element length in bits.
    uint16_t nbits;    ///< Subgroup order length in bits.
    uint8_t ftype;     ///< Field type in hardware.
    uint8_t h;         ///< Co-factor.
    uint8_t len;       ///< Field element length in bytes.
    bool pf;           ///< Prime field flag
} pufs_ecc_param_st;
/**
 * @brief Maximum field element length in bytes.
 */
#ifndef QLEN_MAX
#define QLEN_MAX 72
#endif
/**
 * @brief Elliptic curve point (x,y).
 */
typedef struct {
    uint32_t qlen;       ///< Field element length in bytes.
    uint8_t x[QLEN_MAX]; ///< x-coordinate
    uint8_t y[QLEN_MAX]; ///< y-coordinate
} pufs_ec_point_st;
/**
 * @brief Maximum field element length in bytes.
 */
#ifndef NLEN_MAX
#define NLEN_MAX 72
#endif
/**
 * @brief ECDSA signature (r,s).
 */
typedef struct {
    uint32_t qlen;       ///< Field element length in bytes.
    uint8_t r[NLEN_MAX]; ///< r
    uint8_t s[NLEN_MAX]; ///< s
} pufs_ecdsa_sig_st;

/*****************************************************************************
 * Variables
 ****************************************************************************/
extern pufs_ecc_param_st ecc_param[];

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_ECC_H__*/
