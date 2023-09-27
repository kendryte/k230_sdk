/**
 * @file      pufs_sm2.c
 * @brief     PUFsecurity SM2 API implementation
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
/*
# SM2 Crypto Module

The firmware implements SM2 algorithm based on GM/T 0003.1 standard.

## Dependent Modules
1. DMA
2. KDF
3. ECP(PKC)
4. HMAC

## Impelementations
1. SM2 encryption
2. SM2 decryption
3. SM2 digital signature algorithm
4. SM2 key exchange protocol

*/
#include "pufs_internal.h"
#include "pufs_dma_internal.h"
#include "pufs_sm2_internal.h"
#include "pufs_ecp_internal.h"
#include "pufs_hmac_internal.h"
#include "pufs_ecp_regs.h"

/*****************************************************************************
 * Static functions
 ****************************************************************************/

// ZA = H256(ENTLA || id || a || b || Gx || Gy || PUKx || PUKy)
static pufs_status_t pufs_sm2_gen_z(pufs_dgst_st *md,
                                    const uint8_t* id,
                                    uint32_t idlen,
                                    pufs_ec_point_st *puk)
{
    uint8_t n;
    pufs_status_t status;
    pufs_ecc_param_st sm2_params = ecc_param[SM2];

    uint16_t bits = idlen * 8;

    if ((status = cb_pufs_hash_init(SM3)) != SUCCESS)
        goto release;

    n = (uint8_t)((bits >> 8) & 0xFF);
    if ((status = cb_pufs_hash_update(&n, 1)) != SUCCESS)
        goto release;
    
    n = (uint8_t)(bits & 0xFF);
    if ((status = cb_pufs_hash_update( &n, 1)) != SUCCESS)
        goto release;
    
    if (idlen > 0 && (status = cb_pufs_hash_update(id, idlen)) != SUCCESS)
        goto release;
    
    if ((status = cb_pufs_hash_update(sm2_params.a, sm2_params.len)) != SUCCESS)
        goto release;
    
    if ((status = cb_pufs_hash_update(sm2_params.b, sm2_params.len)) != SUCCESS)
        goto release;
    
    if ((status = cb_pufs_hash_update(sm2_params.px, sm2_params.len)) != SUCCESS)
        goto release;
    
    if ((status = cb_pufs_hash_update(sm2_params.py, sm2_params.len)) != SUCCESS)
        goto release;
    
    if ((status = cb_pufs_hash_update(puk->x, puk->qlen)) != SUCCESS)
        goto release;
    
    if ((status = cb_pufs_hash_update(puk->y, puk->qlen)) != SUCCESS)
        goto release;

    status = cb_pufs_hash_final(md);

release:

    return status;
}

// compute SM3_HASH(Za | M)
static pufs_status_t pufs_sm2_sign_m_hash(pufs_dgst_st *md,
                                         pufs_dgst_st *za,
                                         const uint8_t *msg,
                                         uint32_t msg_len)
{
    pufs_status_t status;

    if ((status = cb_pufs_hash_init(SM3)) != SUCCESS)
        goto release;

    if ((status = cb_pufs_hash_update(za->dgst, za->dlen)) != SUCCESS)
        goto release;
    
    if ((status = cb_pufs_hash_update(msg, msg_len)) != SUCCESS)
        goto release;
    
    status = cb_pufs_hash_final(md);

release:

    return status;
}

/**
 * pufs_sm2_verify
 */
static pufs_status_t pufs_sm2_verify(pufs_ecdsa_sig_st sig,
                              const uint8_t* msg,
                              uint32_t msglen,
                              const uint8_t* id,
                              uint32_t idlen,
                              pufs_ec_point_st puk)
{
    pufs_status_t status;
    pufs_dgst_st za, md;

    if ((status = cb_pufs_ecp_set_curve_byname(SM2)) != SUCCESS ||
        (status = cb_pufs_sm2_gen_z(&za, id, idlen, &puk)) != SUCCESS ||
        (status = cb_pufs_sm2_sign_m_hash(&md, &za, msg, msglen)) != SUCCESS ||
        (status = cb_pufs_ecp_sm2_verify_dgst(&sig, &md, &puk)) != SUCCESS)
    {

    }
    return status;
}

static pufs_status_t pufs_sm2_verify_lock(pufs_ecdsa_sig_st sig,
                              const uint8_t* msg,
                              uint32_t msglen,
                              const uint8_t* id,
                              uint32_t idlen,
                              pufs_ec_point_st puk)
{
    pufs_status_t ret=0;
    hardlock_config(lock);
    ret = pufs_sm2_verify(sig, msg, msglen, id, idlen, puk);
    hardlock_config(unlock);
    return ret;
}

func_pufs_sm2_gen_z cb_pufs_sm2_gen_z = pufs_sm2_gen_z;
func_pufs_sm2_sign_m_hash cb_pufs_sm2_sign_m_hash = pufs_sm2_sign_m_hash;
func_pufs_sm2_verify cb_pufs_sm2_verify = pufs_sm2_verify_lock;