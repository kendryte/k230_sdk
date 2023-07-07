/**
 * @file      pufs_ecp.c
 * @brief     PUFsecurity ECP API implementation
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
#include "pufs_ecp_internal.h"
#include "pufs_ecp_mprog.h"
#include "pufs_ka_internal.h"
#include "pufs_ecc_internal.h"
#include "platform.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")
static pufs_ecp_version_t ecp_version = 0;
static pufs_mp_version_t mp_version = 0;



struct pufs_ecp_regs *ecp_regs = (struct pufs_ecp_regs *)(PUFIOT_ADDR_START+PKC_ADDR_OFFSET);

/*****************************************************************************
 * Macros
 ****************************************************************************/
#define MAXELEN 72
#define ECP_MPMAC_SIZE 16
#define ECP_MPROG_SIZE 256

/*****************************************************************************
 * Static variables
 ****************************************************************************/





struct ecdp_setting
{
    pufs_ecp_mprog_curve_st* mprog;
    pufs_ecp_mprog_cmac_st* mpmac;
    pufs_ec_name_t name;
    bool isset;
} ecdp_set;

/*****************************************************************************
 * Static functions
 ****************************************************************************/
static void write_ecp_operand(uint32_t pos, const uint8_t* op, uint32_t elen)
{
    uint32_t wlen = ((elen + 3) / 4) * 4;

    if (op != NULL)
        cb_reverse(pufs_buffer, op, elen);
    memcpy((uint8_t *)ecp_regs->data + wlen * pos, pufs_buffer, wlen);
}

/**
 * @brief Read resulting operands from SRAM
 *
 * @param pos       The result position of the operand to be read from SRAM
 * @param res       The pointer to the space for the resulting operand
 * @param elen      The length in bytes of the resulting operand stored in SRAM
 * @prarm oss_2e2s  Indication of 2e2s_oss read.
 */
static void read_ecp_operand(uint32_t pos, uint8_t* res, uint32_t elen,
                             bool oss_2e2s)
{
    uint32_t wlen = ((elen + 3) / 4) * 4;
    uint32_t rlen = wlen;
    if (oss_2e2s)
        rlen = ((elen * 2 + 3) / 4) * 4;

    memcpy(pufs_buffer, (uint8_t *)ecp_regs->data + wlen * pos, rlen);
    cb_reverse(res, pufs_buffer, (oss_2e2s ? (elen * 2) : elen));
}
/**
 * @brief Starting ECP and wait until done
 *
 * @return ECCA status register value
 */
static uint32_t pufs_ecc_start(void)
{
    uint32_t ret;
    ecp_regs->ctrl = 0x01;
    do
    {
        ret = ecp_regs->status;
    }while((ret & ECP_STATUS_BUSY_MASK) != 0);

    return ret;
}

/**
 * @brief Bignum comparison of a and b.
 *
 * @param[in]  a    a.
 * @param[in]  b    b.
 * @param[in]  len  a, b length in bytes.
 * @return          An integral value which is greater than, equal to, or less
 *                   then 0 representing a > b, a = b, or a < b.
 */
static int16_t pufs_bn_cmp(const uint8_t* a, const uint8_t* b, uint32_t len)
{
    for (uint32_t i=0; i<len; i++)
    {
        if (a[i] == b[i])
            continue;
        return ((int16_t)(a[i]) - (int16_t)(b[i]));
    }
    return 0;
}
/**
 * @brief Calculate RSA exponentiation with public key
 *
 * @param[out] msg      Message.
 * @param[in]  sig      RSA signature.
 * @param[in]  rsatype  RSA type.
 * @param[in]  n        RSA parameter n.
 * @param[in]  puk      RSA public key.
 * @return              SUCCESS on success, otherwise an error code.
 */
static pufs_status_t pufs_rsa_verify_calc(uint8_t* msg,
                                          const uint8_t* sig,
                                          pufs_rsa_type_t rsatype,
                                          const uint8_t* n,
                                          uint32_t puk)
{
    uint32_t elen = 0, val32 = 0;
    pufs_ecp_field_t field;

    elen = 256;
    field = P2048;

    if (cb_pufs_bn_cmp(sig, n, elen) >= 0)
        return E_INVALID;

    val32 = field << ECP_ECP_EC_FIELD_BITS;

    ecp_regs->ec = val32;
    ecp_regs->e_short = puk;

    cb_write_ecp_operand(0, n, elen);
    cb_write_ecp_operand(2, sig, elen);

    pufs_rsa_mprog_st *prog = &(rsa_mprog[ecp_version][rsatype]);
    pufs_rsa_mprog_cmac_st *cmac = prog->cmac[mp_version];

    memcpy((void *)ecp_regs->mac, cmac->puk, ECP_MPMAC_SIZE);

    memcpy((void *)ecp_regs->program, prog->func->puk, ECP_MPROG_SIZE);

    val32 = cb_pufs_ecc_start();

    if (val32 & ECP_STATUS_MPROG_MASK)
        return E_ECMPROG;

    if (val32)
        return E_VERFAIL;

    memset(msg, 0, BUFFER_SIZE);
    cb_read_ecp_operand(2, msg, elen, false);

    return SUCCESS;
}

static pufs_status_t pufs_ecp_set_curve_params_byname(pufs_ec_name_t name, uint32_t ecp_ver)
{
    uint32_t val32, elen;

    elen = ecc_param[name].len;
    cb_write_ecp_operand(0, ecc_param[name].field, elen);
    cb_write_ecp_operand(1, ecc_param[name].a, elen);
    cb_write_ecp_operand(2, ecc_param[name].b, elen);
    cb_write_ecp_operand(3, ecc_param[name].px, elen);
    cb_write_ecp_operand(4, ecc_param[name].py, elen);
    cb_write_ecp_operand(5, ecc_param[name].order, elen);

    val32 = ecc_param[name].h<<16 | ecc_param[name].pf<<15 | ecc_param[name].ftype<<8;

    ecp_regs->ec = val32;

    return SUCCESS;
}

/**
 * pufs_ecp_set_sm2_curve()
 */
pufs_status_t pufs_ecp_set_sm2_curve(void)
{
    ecdp_set.name = SM2;
    ecdp_set.isset = true;

    return cb_pufs_ecp_set_curve_params_byname(SM2, ecp_regs->version);
}

pufs_status_t pufs_ecp_sm2_verify_dgst(pufs_ecdsa_sig_st *sig,
                                       pufs_dgst_st *md,
                                       pufs_ec_point_st *puk)
{
    uint32_t val32, elen;

    if (ecdp_set.isset == false)
        return E_INVALID;

    elen = ecc_param[ecdp_set.name].len;
    if ((puk->qlen != elen) || (sig->qlen != elen))
        return E_INVALID;

    if (ecp_regs->status & ECP_STATUS_BUSY_MASK)
        return E_BUSY;

    pufs_sm2_mprog_st *prog = &(sm2_mprog[ecp_version]);
    pufs_sm2_mprog_cmac_st *cmac = prog->cmac[mp_version];

    memcpy((void *)ecp_regs->mac, cmac->verify, ECP_MPMAC_SIZE);

    memcpy((void *)ecp_regs->program, prog->func->verify, ECP_MPROG_SIZE);

    cb_write_ecp_operand(6, md->dgst, md->dlen);
    cb_write_ecp_operand(7, puk->x, elen);
    cb_write_ecp_operand(8, puk->y, elen);
    cb_write_ecp_operand(9, sig->r, elen);
    cb_write_ecp_operand(10, sig->s, elen);

    val32 = cb_pufs_ecc_start();

    if (val32 & ECP_STATUS_MPROG_MASK)
        return E_ECMPROG;

    if (val32)
        return E_VERFAIL;

    return SUCCESS;
}


/**
 * pufs_ecp_set_curve_byname()
 */
pufs_status_t pufs_ecp_set_curve_byname(pufs_ec_name_t name)
{
    if ((ecdp_set.name == name) && ecdp_set.isset)
        return SUCCESS;
    if (ecp_regs->status & ECP_STATUS_BUSY_MASK)
        return E_BUSY;

    return cb_pufs_ecp_set_sm2_curve();
}

pufs_status_t pufs_rsa_p1v15_verify(const uint8_t* sig,
                                    pufs_rsa_type_t rsatype,
                                    const uint8_t* n,
                                    uint32_t puk,
                                    const uint8_t* msg,
                                    uint32_t msglen)
{
    pufs_status_t check;
    uint8_t em[BUFFER_SIZE];
    uint32_t elen, i;
    pufs_hash_t hash;
    pufs_dgst_st md;
    uint8_t pret[19] = { 0x30, 0, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48,
                         0x01, 0x65, 0x03, 0x04, 0x02, 0, 0x05, 0x00, 0x04, 0 };

    elen = 256;

    if ((check = cb_pufs_rsa_verify_calc(em, sig, rsatype, n, puk)) != SUCCESS)
        return check;

    if ((em[0] != 0x00) || (em[1] != 0x01))
        return E_VERFAIL;
    for (i=2; i<elen; i++)
        if (em[i] != 0xff)
            break;
    if (em[i++] != 0x00)
        return E_VERFAIL;

    switch (em[i+14])
    {
    case 1:
        hash = SHA_256; pret[1] = 0x31; pret[14] = 0x01; pret[18] = 0x20;
        break;
    case 2:
        hash = SHA_384; pret[1] = 0x41; pret[14] = 0x02; pret[18] = 0x30;
        break;
    case 3:
        hash = SHA_512; pret[1] = 0x51; pret[14] = 0x03; pret[18] = 0x40;
        break;
    case 4:
        hash = SHA_224; pret[1] = 0x2d; pret[14] = 0x04; pret[18] = 0x1c;
        break;
    case 5:
        hash = SHA_512_224; pret[1] = 0x2d; pret[14] = 0x05; pret[18] = 0x1c;
        break;
    case 6:
        hash = SHA_512_256; pret[1] = 0x31; pret[14] = 0x06; pret[18] = 0x20;
        break;
    default:
        return E_INVALID;
    }
    if ((memcmp(em + i, pret, 19) != 0) || ((i + 19 + pret[18]) != elen))
        return E_VERFAIL;

    if ((check = cb_pufs_hash(&md, msg, msglen, hash)) != SUCCESS)
        return check;
    if (memcmp(em + i + 19, md.dgst, md.dlen) != 0)
        return E_VERFAIL;

    return SUCCESS;
}

func_write_ecp_operand cb_write_ecp_operand = write_ecp_operand;
func_read_ecp_operand cb_read_ecp_operand = read_ecp_operand;
func_pufs_ecc_start cb_pufs_ecc_start = pufs_ecc_start;
func_pufs_bn_cmp cb_pufs_bn_cmp = pufs_bn_cmp;
func_pufs_rsa_verify_calc cb_pufs_rsa_verify_calc = pufs_rsa_verify_calc;
func_pufs_ecp_set_curve_params_byname cb_pufs_ecp_set_curve_params_byname = pufs_ecp_set_curve_params_byname;
func_pufs_ecp_set_sm2_curve cb_pufs_ecp_set_sm2_curve = pufs_ecp_set_sm2_curve;
func_pufs_ecp_sm2_verify_dgst cb_pufs_ecp_sm2_verify_dgst = pufs_ecp_sm2_verify_dgst;
func_pufs_ecp_set_curve_byname cb_pufs_ecp_set_curve_byname = pufs_ecp_set_curve_byname;
func_pufs_rsa_p1v15_verify cb_pufs_rsa_p1v15_verify = pufs_rsa_p1v15_verify;
#pragma GCC pop_options