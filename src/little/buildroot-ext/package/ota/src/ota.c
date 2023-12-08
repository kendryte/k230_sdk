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
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/reboot.h>
#include "rsa_sign.h"

#define CONFIG_FILE_PATH "/etc/ota.conf"
#define OTA_PACKAGE_PATH "/tmp/ota_package.kpk"
#define OTA_FIRMWARE_PATH "/tmp/ota_package.bin"
#define PUBLIC_KEY_PATH "/tmp/ota_public.pem"
#define SCRIPT_PATH "/tmp/script.bin"

#define SIGN_SIZE   (256)

typedef struct
{
	char key[64];
	char val[64];
} key_val;
key_val kv[16];

int extract_sign(const char *package_path, uint8_t *sign, int len)
{
    int ret = 0;
    int nread = 0;
    FILE *fp = NULL;

    fp = fopen(package_path, "r");
    if (fp == NULL) {
        printf("fopen %s error \n", package_path);
        return -1;
    }

    nread = fread(sign, 1, len, fp);
    if (nread <= 0) {
        printf("fread %s error \n", package_path);
        return -1;
    }

    ret = fclose(fp);
    if (ret != 0) {
        printf("fclose %s error \n", package_path);
        return -1;
    }

    return nread;
}

int extract_firmware(const char *package_path, const char *firmware_path)
{
    int ret = 0;
    int nread = 0;
    int nwritten = 0;
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
    char buf[4096];

    fp1 = fopen(package_path, "r");
    if (fp1 == NULL) {
        printf("fopen %s error \n", package_path);
        return -1;
    }

    //skip SIGN_SIZE byte rsa sign
    ret = fseek(fp1, SIGN_SIZE, SEEK_SET);
    if (ret < 0) {
        printf("fseek %s error \n", package_path);
        return -1;
    }

    fp2 = fopen(firmware_path, "w+");
    if (fp2 == NULL) {
        printf("fopen %s error \n", firmware_path);
        return -1;
    }

    while (1) {
        memset(buf, 0x00, sizeof(buf));
        nread = fread(buf, 1, sizeof(buf), fp1);
        if (nread == -1) {
            printf("fread %s error \n", package_path);
            return -1;
        } else if (nread == 0) {
            break;
        }

        nwritten = fwrite(buf, 1, nread, fp2);
        if (nwritten == -1) {
            printf("fwrite %s error \n", firmware_path);
            return -1;
        }
    }

    ret = fclose(fp1);
    if (ret != 0) {
        printf("fclose %s error \n", package_path);
        return -1;
    }

    ret = fclose(fp2);
    if (ret != 0) {
        printf("fclose %s error \n", firmware_path);
        return -1;
    }

    return 0;
}

int run_command(const char *command)
{
    int ret = 0;

    ret = system(command);
    if ((WIFEXITED(ret)) && (WEXITSTATUS(ret) != 0)) {
        printf("system %s error \n", command);
        return -1;
    } else if ((WIFEXITED(ret)) && (WEXITSTATUS(ret) == 0)) {
        printf("system %s success \n", command);
        return 0;
    } else if ((WIFEXITED(ret) == 0) && (WEXITSTATUS(ret) == 255)) {
        printf("system lost the SIGCHLD signal \n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;
    char tmp[128];
    int i = 0;
    char server_url[64];
    char public_key[64];
    memset(server_url, '\0', sizeof(server_url));
    memset(public_key, '\0', sizeof(public_key));
    // 解析配置文件，获取服务器信息
    FILE *fp = fopen(CONFIG_FILE_PATH, "r");
	if(!fp)
	{
		printf("open config file failed\n");
		return -1;
	}
    while (fgets(tmp, 128, fp))
	{
		if (strstr(tmp, "="))
		{
			sscanf(tmp, "%[^=]=%s", kv[i].key, kv[i].val);
            if(!strcmp(kv[i].key, "server_url"))
            {
                strcpy(server_url, kv[i].val);
            }
            else if(!strcmp(kv[i].key, "public_key"))
            {
                strcpy(public_key, kv[i].val);
            }
            i ++;
		}
    }
    fclose(fp);

    // wget 从服务器下载文件，下载失败退出
    char command[8192];
    memset(command, 0x00, sizeof(command));
    snprintf(command, sizeof(command), "wget -nv -O %s %s",
            OTA_PACKAGE_PATH,
            server_url);

    ret = run_command(command);
    if (ret < 0) {
        printf("run_command %s error \n", command);
        return -1;
    }

    // 分离签名与升级包
    uint8_t sign[SIGN_SIZE];
    unsigned int nread = 0;
    memset(sign, 0x00, SIGN_SIZE);
    nread = extract_sign(OTA_PACKAGE_PATH, sign, SIGN_SIZE);
    if (nread != SIGN_SIZE) {
        printf("extract sign error \n");
        return -1;
    }
    ret = extract_firmware(OTA_PACKAGE_PATH, OTA_FIRMWARE_PATH);
    if (ret < 0) {
        printf("extract firmware error \n");
        return -1;
    }
    ret = chmod(OTA_FIRMWARE_PATH, 0774);
    if (ret < 0) {
        printf("chmod %s error \n", OTA_FIRMWARE_PATH);
        return -1;
    }

    // 验签，失败删除下载文件，退出
    static int digest_type = NID_md5;
    ret = verify_sign(OTA_FIRMWARE_PATH, public_key, digest_type, sign, nread);
    if (ret < 0) {
        printf("OTA verify sign error \n");
        return -1;
    } else if (ret == 0) {
        printf("OTA verify sign success \n");
    }
    // 内存不够，考虑先分离脚本与固件，然后删除升级包
    memset(command, 0x00, sizeof(command));
    snprintf(command, sizeof(command), "%s 0",
            OTA_FIRMWARE_PATH);
    ret = run_command(command);
    if (ret < 0) {
        printf("run_command %s error \n", command);
        return -1;
    }
    ret = chmod(SCRIPT_PATH, 0774);
    if (ret < 0) {
        printf("chmod %s error \n", SCRIPT_PATH);
        return -1;
    }
    // popen执行升级包前的脚本
    int ota_reboot_flag = 0;
    char pbuff[256];
    memset(pbuff, 0x00, sizeof(pbuff));
    memset(command, 0x00, sizeof(command));
    snprintf(command, sizeof(command), "%s 1",
            SCRIPT_PATH);
    ret = run_command(command);
    if (ret < 0) {
        printf("run_command %s error \n", command);
        return -1;
    }
    ota_reboot_flag = 1;
    // while (fgets(pbuff, sizeof(pbuff), fp)) {
    //     // no need to compare the last character '\n'
    //     if (!strncmp(pbuff, "decompress ota_package.zip success", strlen(pbuff)-1)) {
    //         printf("OTA update progress 50% \n");
    //     } else if (!strncmp(pbuff, "OTA update success", strlen(pbuff)-1)) {
    //         printf("OTA update success \n");
    //         ota_reboot_flag = 1;
    //     } else {
    //         continue;
    //     }
    //     memset(pbuff, 0x00, sizeof(pbuff));
    // }
    // ret = pclose(fp);
    // if ((WIFEXITED(ret)) && (WEXITSTATUS(ret) != 0)) {
    //     printf("OTA update error \n");
    //     printf("run %s error \n", OTA_FIRMWARE_PATH);
    //     return -1;
    // } else if ((WIFEXITED(ret)) && (WEXITSTATUS(ret) == 0)) {
    //     printf("We captured the SIGCHLD signal \n");
    // } else if ((WIFEXITED(ret) == 0) && (WEXITSTATUS(ret) == 255)) {
    //     printf("We lost the SIGCHLD signal \n");
    // }

    // 重启系统
    if (ota_reboot_flag == 1) {
        printf("OTA will delete files \n");
        sync();
        sync();
        printf("OTA will reboot system \n");
        sleep(10);
        sync();
        sync();
        reboot(RB_AUTOBOOT);
    }

    return 0;
}