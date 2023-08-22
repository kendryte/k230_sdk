#ifndef __RSA_SIGN_H_
#define __RSA_SIGN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>

int gen_sign(const char *file_path, const char *key_path, int digest_type, unsigned char *sign, unsigned int *len);
int verify_sign(const char *file_path, const char *key_path, int digest_type, unsigned char *sign, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* __RSA_SIGN_H_ */
