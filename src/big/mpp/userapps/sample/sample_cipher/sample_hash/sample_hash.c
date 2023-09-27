/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>

#include "mpi_cipher_api.h"

int main(void)
{
    k_s32 ret = K_ERR_OK;

    k_char *msg[] = {
        "",

        "abc",

        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",

        "The quick brown fox jumps over the lazy dog",

        "The quick brown fox jumps over the lazy cog",

        "bhn5bjmoniertqea40wro2upyflkydsibsk8ylkmgbvwi420t44cq034eou1szc1k0mk46oeb7ktzmlxqkbte2sy",
    };

    k_char *golden_hash[] = {
        "e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855",
        "ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad",
        "248d6a61 d20638b8 e5c02693 0c3e6039 a33ce459 64ff2167 f6ecedd4 19db06c1",
        "d7a8fbb3 07d78094 69ca9abc b0082e4f 8d5651e4 6d3cdb76 2d02d0bf 37c9e592",
        "e4c4d8f3 bf76b692 de791a17 3e053211 50f7a345 b46484fe 427f6acc 7ecc81be",
        "9085df2f 02e0cc45 5928d0f5 1b27b4bf 1d9cd260 a66ed1fd a11b0a3f f5756d99",
    };

    k_char compare_golden_hash[6][SHA256_HASH_SIZE] = {{0xe3,0xb0,0xc4,0x42,0x98,0xfc,0x1c,0x14,0x9a,0xfb,0xf4,0xc8,0x99,0x6f,0xb9,0x24,0x27,0xae,0x41,0xe4,0x64,0x9b,0x93,0x4c,0xa4,0x95,0x99,0x1b,0x78,0x52,0xb8,0x55},
                                            {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad}, 
                                            {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1},
                                            {0xd7,0xa8,0xfb,0xb3,0x07,0xd7,0x80,0x94,0x69,0xca,0x9a,0xbc,0xb0,0x08,0x2e,0x4f,0x8d,0x56,0x51,0xe4,0x6d,0x3c,0xdb,0x76,0x2d,0x02,0xd0,0xbf,0x37,0xc9,0xe5,0x92},
                                            {0xe4,0xc4,0xd8,0xf3,0xbf,0x76,0xb6,0x92,0xde,0x79,0x1a,0x17,0x3e,0x05,0x32,0x11,0x50,0xf7,0xa3,0x45,0xb4,0x64,0x84,0xfe,0x42,0x7f,0x6a,0xcc,0x7e,0xcc,0x81,0xbe},
                                            {0x90,0x85,0xdf,0x2f,0x02,0xe0,0xcc,0x45,0x59,0x28,0xd0,0xf5,0x1b,0x27,0xb4,0xbf,0x1d,0x9c,0xd2,0x60,0xa6,0x6e,0xd1,0xfd,0xa1,0x1b,0x0a,0x3f,0xf5,0x75,0x6d,0x99}};


    const k_u32 tests_total = sizeof(msg) / sizeof(msg[0]);
    k_u8 hash[6][SHA256_HASH_SIZE] = {{0}};

    for (k_u32 i = 0; i < tests_total; i++) {
        ret = kd_mpi_cipher_sha256(msg[i], strlen(msg[i]), hash[i]);
        if(ret)
        {
            printf("sha256 failed ret:%d\n", ret);
            return K_FAILED;
        }

        printf("input = '%s'\ndigest: %s\nresult: ", msg[i], golden_hash[i]);

        if (memcmp(compare_golden_hash[i], hash[i], SHA256_HASH_SIZE) == 0) {
            printf("PASS\n\n");
        } else {
            printf("FAIL\n\n");
        }

        printf("\n\n");
    }

    return K_SUCCESS;
}