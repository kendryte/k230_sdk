/**
 * @file      pufs_ecp_mprog.h
 * @brief     PUFsecurity ECP micro program
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

#ifndef __PUFS_ECP_MPROG_H__
#define __PUFS_ECP_MPROG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "pufs_ecc.h"

/*****************************************************************************
 * Structures
 ****************************************************************************/
/**
 * @brief CMAC of RSA micro programs
 */
typedef struct
{
    const void *prk;
    const void *puk;
} pufs_rsa_mprog_cmac_st;

/**
 * @brief Functions of RSA micro programs
 */
typedef struct
{
    const void *prk;
    const void *puk;
} pufs_rsa_mprog_func_st;

/**
 * @brief RSA micro programs
 */
 typedef struct
{
    pufs_rsa_mprog_cmac_st **cmac;
    pufs_rsa_mprog_func_st *func;
} pufs_rsa_mprog_st;

/**
 * @brief CMAC of SM2 micro programs
 */
typedef struct
{
    const void *enc_oss;
    const void *dec_oss;
    const void *puk_gen;
    const void *prks_gen;
    const void *sign;
    const void *verify;
    const void *kekdf;
} pufs_sm2_mprog_cmac_st;

/**
 * @brief Functions of SM2 micro programs
 */
typedef struct
{
    const void *enc_oss;
    const void *dec_oss;
    const void *puk_gen;
    const void *prks_gen;
    const void *sign;
    const void *verify;
    const void *kekdf;
} pufs_sm2_mprog_func_st;

/**
 * @brief RSA micro programs
 */
 typedef struct
{
    pufs_sm2_mprog_cmac_st **cmac;
    pufs_sm2_mprog_func_st *func;
} pufs_sm2_mprog_st;

/**
 * @brief CMAC of ECP micro programs
 */
typedef struct
{
    uint32_t mp_version;
    const void* eccdh_2e;
    const void* eccdh_2e_oss;
    const void* eccdh_2s2e;
    const void* eccdh_2s2e_oss;
    const void* ecdsa_s;
    const void* ecdsa_s_ik;
    const void* ecdsa_v;
    const void* ecdsa_v_otpk;
    const void* prke_gen;
    const void* prki_gen;
    const void* prks_gen;
    const void* puk_gen;
    const void* pukv_f;
    const void* pukv_p;
} pufs_ecp_mprog_cmac_st;
/**
 * @brief ECP micro programs of a curve
 */
typedef struct
{
    pufs_ec_name_t name;
    const void* eccdh_2e;
    const void* eccdh_2e_oss;
    const void* eccdh_2s2e;
    const void* eccdh_2s2e_oss;
    const void* ecdsa_s;
    const void* ecdsa_s_ik;
    const void* ecdsa_v;
    const void* ecdsa_v_otpk;
    const void* prke_gen;
    const void* prki_gen;
    const void* prks_gen;
    const void* puk_gen;
    const void* pukv_f;
    const void* pukv_p;
    pufs_ecp_mprog_cmac_st** mprog_sum;
} pufs_ecp_mprog_curve_st;
/**
 * @brief ECP micro programs
 */
typedef struct
{
    uint32_t ecp_version;
    pufs_ecp_mprog_curve_st** mprog;
} pufs_ecp_mprog_st;

/*****************************************************************************
 * Variables
 ****************************************************************************/
extern pufs_ecp_mprog_st* ecp_mprog[];

extern pufs_rsa_mprog_st* rsa_mprog[];

extern pufs_sm2_mprog_st sm2_mprog[];

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /*__PUFS_ECP_MPROG_H__*/
