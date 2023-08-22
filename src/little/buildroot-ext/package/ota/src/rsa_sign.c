#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "rsa_sign.h"

#define BUF_SIZE            (1024)
#define MD_SIZE             (128)

int print_hex(unsigned char *hex, int len)
{
    int i = 0;

    for (i = 0; i < len; i ++)
        printf("%02x", hex[i]);
    printf("\n");

    return 0;
}

int md5_digest(const char *file_path, unsigned char *md)
{
    FILE *fp = NULL;
    MD5_CTX ctx;
    unsigned char buf[BUF_SIZE];
    size_t nread = 0;

    fp = fopen(file_path, "rb");
    if (fp == NULL) {
        printf("%s: fopen %s error \n", __func__, file_path);
        return -1;
    }

    MD5_Init(&ctx);

    while (1) {
        memset(buf, 0x00, BUF_SIZE);
        nread = fread(buf, 1, BUF_SIZE, fp);
        if (nread <= 0)
            break;
        MD5_Update(&ctx, buf, nread);
    }

    MD5_Final(md, &ctx);
    return fclose(fp);
}

int sha256_digest(const char *file_path, unsigned char *md)
{
    FILE *fp = NULL;
    SHA256_CTX ctx;
    unsigned char buf[BUF_SIZE];
    size_t nread = 0;

    fp = fopen(file_path, "rb");
    if (fp == NULL) {
        printf("%s: fopen %s error \n", __func__, file_path);
        return -1;
    }

    SHA256_Init(&ctx);

    while (1) {
        memset(buf, 0x00, BUF_SIZE);
        nread = fread(buf, 1, BUF_SIZE, fp);
        if (nread <= 0)
            break;
        SHA256_Update(&ctx, buf, nread);
    }

    SHA256_Final(md, &ctx);
    return fclose(fp);
}

RSA *read_private_key(const char *key_path)
{
    FILE *fp = NULL;
    RSA *private_key = NULL;
    int ret = 0;

    fp = fopen(key_path, "r");
    if (fp == NULL) {
        printf("%s: fopen %s error \n", __func__, key_path);
        return NULL;
    }

    private_key = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    if (private_key == NULL) {
        printf("%s: PEM_read_RSAPrivateKey error \n", __func__);
        fclose(fp);
        return NULL;
    }

    ret = fclose(fp);
    if (ret < 0) {
        printf("%s: fclose %s error \n", __func__, key_path);
        return NULL;
    }

    return private_key;
}

RSA *read_public_key(const char *key_path)
{
    FILE *fp = NULL;
    RSA *public_key = NULL;
    int ret = 0;

    fp = fopen(key_path, "r");
    if (fp == NULL) {
        printf("%s: fopen %s error \n", __func__, key_path);
        return NULL;
    }

    public_key = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    if (public_key == NULL) {
        printf("%s: PEM_read_RSA_PUBKEY error \n", __func__);
        fclose(fp);
        return NULL;
    }

    ret = fclose(fp);
    if (ret < 0) {
        printf("%s: fclose %s error \n", __func__, key_path);
        return NULL;
    }

    return public_key;
}

int gen_sign(const char *file_path, const char *key_path, int digest_type, unsigned char *sign, unsigned int *len)
{
    uint8_t md[MD_SIZE];
    RSA *private_key = NULL;
    unsigned int digest_length = 0;
    int ret = 0;

    memset(md, 0x00, MD_SIZE);
    if (digest_type == NID_md5) {
        digest_length = MD5_DIGEST_LENGTH;
        ret = md5_digest(file_path, md);
        if (ret < 0)
            return -1;
    } else if (digest_type == NID_sha256) {
        digest_length = SHA256_DIGEST_LENGTH;
        ret = sha256_digest(file_path, md);
        if (ret < 0)
            return -1;
    } else {
        printf("%s: digest_type error \n", __func__);
        return -1;
    }

    private_key = read_private_key(key_path);
    if (private_key == NULL) {
        printf("%s: read_private_key error \n", __func__);
        return -1;
    }

    ret = RSA_sign(digest_type, md, digest_length, sign, len, private_key);
    if (ret < 0) {
        printf("%s: RSA_sign error \n", __func__);
        RSA_free(private_key);
        return -1;
    }

    RSA_free(private_key);

    return 0;
}

int verify_sign(const char *file_path, const char *key_path, int digest_type, unsigned char *sign, unsigned int len)
{
    uint8_t md[MD_SIZE];
    RSA *public_key = NULL;
    unsigned int digest_length = 0;
    int ret = 0;

    memset(md, 0x00, MD_SIZE);
    if (digest_type == NID_md5) {
        digest_length = MD5_DIGEST_LENGTH;
        ret = md5_digest(file_path, md);
        if (ret < 0)
            return -1;
    } else if (digest_type == NID_sha256) {
        digest_length = SHA256_DIGEST_LENGTH;
        ret = sha256_digest(file_path, md);
        if (ret < 0)
            return -1;
    } else {
        printf("%s: digest_type error \n", __func__);
        return -1;
    }

    public_key = read_public_key(key_path);
    if (public_key == NULL) {
        printf("%s: read_public_key error \n", __func__);
        return -1;
    }

    ret = RSA_verify(digest_type, md, digest_length, sign, len, public_key);
    if (ret == 1) {
        printf("%s: RSA_verify success \n", __func__);
        RSA_free(public_key);
        return 0;
    } else {
        printf("%s: RSA_verify fail \n", __func__);
        RSA_free(public_key);
        return -1;
    }

    return 0;
}

