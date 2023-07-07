/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-22     MurphyZhao   the first version
 */

#include <rtthread.h>

#include <tiny_aes.h>

#define TEST_TINY_AES_IV  "0123456789ABCDEF"
#define TEST_TINY_AES_KEY "0123456789ABCDEF0123456789ABCDEF"

static rt_err_t test_tiny_aes(void)
{
    tiny_aes_context ctx;
    uint8_t iv[16 + 1];
    uint8_t private_key[32 + 1];

    uint8_t data[] = "1234567890123456";
    uint8_t data_encrypt[32];
    uint8_t data_decrypt[32];

    /* encrypt */
    rt_memcpy(iv, TEST_TINY_AES_IV, rt_strlen(TEST_TINY_AES_IV));
    iv[sizeof(iv) - 1] = '\0';
    rt_memcpy(private_key, TEST_TINY_AES_KEY, rt_strlen(TEST_TINY_AES_KEY));
    private_key[sizeof(private_key) - 1] = '\0';

    rt_memset(data_encrypt, 0x0, sizeof(data_encrypt));
    tiny_aes_setkey_enc(&ctx, (uint8_t *) private_key, 256);
    tiny_aes_crypt_cbc(&ctx, AES_ENCRYPT, rt_strlen(data), iv, data, data_encrypt);

    /* decrypt */
    rt_memcpy(iv, TEST_TINY_AES_IV, rt_strlen(TEST_TINY_AES_IV));
    iv[sizeof(iv) - 1] = '\0';
    rt_memcpy(private_key, TEST_TINY_AES_KEY, rt_strlen(TEST_TINY_AES_KEY));
    private_key[sizeof(private_key) - 1] = '\0';

    rt_memset(data_decrypt, 0x0, sizeof(data_decrypt));
    tiny_aes_setkey_dec(&ctx, (uint8_t *) private_key, 256);
    tiny_aes_crypt_cbc(&ctx, AES_DECRYPT, rt_strlen(data), iv, data_encrypt, data_decrypt);

    if(rt_memcmp(data, data_decrypt, rt_strlen(data)) == 0)
    {
        rt_kprintf("AES CBC mode passed!\n");
        return RT_EOK;
    }
    else
    {
        rt_kprintf("AES CBC mode failed!");
        return -RT_ERROR;
    }
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(test_tiny_aes, tiny_aes_sample, type tiny_aes_sample cmd to run);
#endif /* FINSH_USING_MSH */
