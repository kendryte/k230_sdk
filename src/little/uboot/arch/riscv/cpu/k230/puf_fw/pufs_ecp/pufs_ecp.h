/**
 * @file      pufs_ecp.h
 * @brief     PUFsecurity ECP API interface
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

#ifndef __PUFS_ECP_H__
#define __PUFS_ECP_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef void* (* func_memset)(void *p, int c,int size);
// extern func_memset memset;

// typedef void* (* func_memcpy4b)(void * dest,const void *src,int count);
// extern func_memcpy4b cb_memcpy4b;

// typedef void* (* func_memcpy)(void * dest,const void *src,int count);
// extern func_memcpy cb_memcpy;

// typedef int (* func_memcmp)(const void * buf0, const void *buf1, int count);
// extern func_memcmp cb_memcmp;


#include <stdbool.h>
#include "pufs_ka.h"
#include "pufs_ecc.h"
#include "pufs_hmac.h"
#include "pufs_rt.h"

typedef void (* func_write_ecp_operand)(uint32_t pos, const uint8_t* op, uint32_t elen);
extern func_write_ecp_operand cb_write_ecp_operand;

typedef void (* func_read_ecp_operand)(uint32_t pos, uint8_t* res, uint32_t elen,
                             bool oss_2e2s);
extern func_read_ecp_operand cb_read_ecp_operand;

typedef uint32_t (* func_pufs_ecc_start)(void);
extern func_pufs_ecc_start cb_pufs_ecc_start;

typedef int16_t (* func_pufs_bn_cmp)(const uint8_t* a, const uint8_t* b, uint32_t len);
extern func_pufs_bn_cmp cb_pufs_bn_cmp;

typedef pufs_status_t (* func_pufs_rsa_verify_calc)(uint8_t* msg,
                                          const uint8_t* sig,
                                          pufs_rsa_type_t rsatype,
                                          const uint8_t* n,
                                          uint32_t puk);
extern func_pufs_rsa_verify_calc cb_pufs_rsa_verify_calc;

typedef pufs_status_t (* func_pufs_ecp_set_curve_params_byname)(pufs_ec_name_t name, uint32_t ecp_ver);
extern func_pufs_ecp_set_curve_params_byname cb_pufs_ecp_set_curve_params_byname;

typedef pufs_status_t (* func_pufs_ecp_set_sm2_curve)(void);
extern func_pufs_ecp_set_sm2_curve cb_pufs_ecp_set_sm2_curve;

typedef pufs_status_t (* func_pufs_ecp_sm2_verify_dgst)(pufs_ecdsa_sig_st *sig,
                                       pufs_dgst_st *md,
                                       pufs_ec_point_st *puk);
extern func_pufs_ecp_sm2_verify_dgst cb_pufs_ecp_sm2_verify_dgst;

typedef pufs_status_t (* func_pufs_ecp_set_curve_byname)(pufs_ec_name_t name);
extern func_pufs_ecp_set_curve_byname cb_pufs_ecp_set_curve_byname;

typedef pufs_status_t (* func_pufs_rsa_p1v15_verify)(const uint8_t* sig,
                                    pufs_rsa_type_t rsatype,
                                    const uint8_t* n,
                                    uint32_t puk,
                                    const uint8_t* msg,
                                    uint32_t msglen);

extern func_pufs_rsa_p1v15_verify cb_pufs_rsa_p1v15_verify;

/**
 * @brief Set elliptic curve domain parameters by name.
 *
 * @param[in] name  Elliptic curve name.
 * @return          SUCCESS on success, otherwise an error code.
 */
typedef pufs_status_t (* func_pufs_ecp_set_curve_byname)(pufs_ec_name_t name);

typedef pufs_status_t (* func_pufs_rsa_p1v15_verify)(const uint8_t* sig,
                                    pufs_rsa_type_t rsatype,
                                    const uint8_t* n,
                                    uint32_t puk,
                                    const uint8_t* msg,
                                    uint32_t msglen);
pufs_status_t pufs_rsa_p1v15_verify(const uint8_t* sig,
                                    pufs_rsa_type_t rsatype,
                                    const uint8_t* n,
                                    uint32_t puk,
                                    const uint8_t* msg,
                                    uint32_t msglen);                                    

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif /* __PUFS_ECP_H__ */
