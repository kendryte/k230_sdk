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

#include "db_proc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include "k_autoconf_comm.h"
#include <sys/socket.h>
#include <linux/if_alg.h>
#define uint32_t uint
#define uint8_t unsigned char

#define ER(func) {int ret=func; if(ret)\
                   {printf("error f=%s l=%d ret=%x\n", __func__, __LINE__,ret );return ret; } } //func error ,return

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;

#define MAGIC_NUM   0x3033324B // "K230"

typedef enum {
    NONE_SECURITY = 0,
    GCM_ONLY,
    CHINESE_SECURITY,
    INTERNATIONAL_SECURITY
} crypto_type_e;


static const struct aes_gcm_para
{
	uint32_t keybits;
	const void *key;
	uint32_t ivlen;
	const void *iv;
	uint32_t aadlen;
	const void *aad;
} aes_gcm = 
{  256, "\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f",
   12, "\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94",
   0, "",
};

typedef struct __firmware_head_st
{
    uint32_t magic; // 方便升级时快速判断固件是否有效。
    uint32_t length; // 从存储介质读到SRAM的数据量
    crypto_type_e crypto_type; // 支持国密或国际加密算法，或支持不加密启动(otp可以控制是否支持)。
    // 设想这样一个场景，如果固件只使用对称加密，在工厂批量生产的时候，解密密钥必然会泄露给工厂。如果使用非对称加密就可以这种问题了，只需要把公钥交给工厂。
    union verify_{ 
        struct rsa_{
            uint8_t n[256];// 非对称加密的验签，防止固件被篡改。同时其HASH值会被烧录到otp。
            uint32_t e;
            uint8_t signature[256];
        } rsa;
        struct sm2_{
            uint32_t idlen;
            uint8_t id[512-32*4];
            uint8_t pukx[32];
            uint8_t puky[32];
            uint8_t r[32];
            uint8_t s[32];
        } sm2;
        struct none_sec_{
            uint8_t signature[32];// 计算HASH保证启动固件的完整性。避免程序异常难以定位原因。
            uint8_t reserved[516-32];
        } none_sec;
    } verify;
}__attribute__((packed, aligned(4))) firmware_head_s; //总的512+16 bytes


#define TMP_FILE_FACE_DB_SAVE_FWH   "/tmp/face_db_fwh"
#define TMP_FILE_FACE_DB_SAVE_UH    "/tmp/face_db_uh"
#define TMP_FILE_FACE_DB_SAVE_VER   "/tmp/face_db_ver"
#define TMP_FILE_FACE_DB_SAVE_ORGDATA "/tmp/face_db_org_data.bin"
#define TMP_FILE_FACE_DB_SAVE_GZIPDATA  "/tmp/face_db_org_data.bin.gz"
#define TMP_FILE_FACE_DB_SAVE_SHA256     "/tmp/face_db_sha256"
#define TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT   "/tmp/face_db_ver_uh_gzd"
#define TMP_FILE_FACE_DB_SAVE_FWh_VER_UH_GZIPDAT  "/tmp/face_db_fwh_ver_uh_gzd"
#define TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT_ENC   "/tmp/face_db_ver_uh_gzd_enc"

static int save_buff_2_file(char *buffer,long len,char *name)
{
   int ret = 0;
   FILE *fp = fopen(name, "wb");
   if (!fp) {
      perror("fopen ");
      return EXIT_FAILURE;
   }
   ret = fwrite(buffer, len, 1, fp);
   if (ret != 1) {
      fprintf(stderr, "fread() failed: %x\n", ret);
      return (EXIT_FAILURE);
   }

   fclose(fp);
   //ret = system("set -e; cd /tmp;gzip ttt.bin;");
   return 0;
}

static int read_file_2_buff(char *buffer, long len, char *name)
{
   int ret = 0;
   FILE *fp = fopen(name, "rb");
   if (!fp) {
      perror("fopen");
      return EXIT_FAILURE;
   }

   ret = fread(buffer, len, 1, fp);
   if (ret != 1) {
      fprintf(stderr, "fread() failed: %x\n", ret);
      return (EXIT_FAILURE);
   }

   fclose(fp);
   return 0;
}

static int cal_aes_gcm(const uint8_t *key,
                        uint32_t keybits,
                        unsigned int op,
                        const uint8_t *in,
                        uint32_t inlen,
                        const uint8_t *aad,
                        uint32_t aadlen,
                        const uint8_t *iv,
                        uint32_t ivlen,
                        uint8_t *out)
{
   int tfmfd;
	int opfd;
	int cmsg_size;
	int out_msg_len = (op == ALG_OP_DECRYPT) ? inlen : (inlen + 16);
	unsigned char *buf = (unsigned char *)malloc(out_msg_len + 256);

	struct sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type = "aead",
		.salg_name = "gcm(aes)",
	};

	// create and bind socket
	tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);
	bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));

	//set socket options: key, AEAD Authentication size
	setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY, key, (keybits >> 3));
	setsockopt(tfmfd, SOL_ALG, ALG_SET_AEAD_AUTHSIZE, NULL, 16);
	//accept connection
	opfd = accept(tfmfd, NULL, 0);

	//Prepare Message
	struct msghdr msg = {};
	struct cmsghdr *cmsg;
	struct iovec iov[2];

	cmsg_size = CMSG_SPACE(4);
	cmsg_size += aadlen ? CMSG_SPACE(4) : 0;
	cmsg_size += ivlen ? CMSG_SPACE(sizeof(struct af_alg_iv) + ivlen) : 0;
    char *cbuf = (char *)malloc(cmsg_size);
    memset(cbuf, 0, cmsg_size);

	msg.msg_control = cbuf;
	msg.msg_controllen = cmsg_size;
	msg.msg_iov = iov;

	//set the Headervalues for the Operation
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_ALG;
	cmsg->cmsg_type = ALG_SET_OP;
	cmsg->cmsg_len = CMSG_LEN(4);
	*(__u32 *)CMSG_DATA(cmsg) = op;

	//set headervalues for aad
	if(aadlen)
	{
		cmsg = CMSG_NXTHDR(&msg, cmsg);
        cmsg->cmsg_level = SOL_ALG;
        cmsg->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
        cmsg->cmsg_len = CMSG_LEN(4);
		unsigned int ad_data = (void *)CMSG_DATA(cmsg);
        *(unsigned int *)CMSG_DATA(cmsg) = aadlen;

		iov[0].iov_base = (void *)aad;
        iov[0].iov_len = aadlen;
        iov[1].iov_base = (void *)in;
        iov[1].iov_len = inlen;
        msg.msg_iovlen = 2;
	}
	else
	{
		iov[0].iov_base = (void *)in;
		iov[0].iov_len = inlen;
		msg.msg_iovlen = 1;
	}

	//set headervalues for iv
	if(ivlen)
	{
		struct af_alg_iv *algiv;

		cmsg = CMSG_NXTHDR(&msg, cmsg);
		cmsg->cmsg_level = SOL_ALG;
		cmsg->cmsg_type = ALG_SET_IV;
		cmsg->cmsg_len = CMSG_LEN(sizeof(*algiv) + ivlen);

		algiv = (void *)CMSG_DATA(cmsg);
		algiv->ivlen = ivlen;
		memcpy(algiv->iv, iv, ivlen);
	}

	// send message
	sendmsg(opfd, &msg, MSG_DONTWAIT);

	if (aadlen)
	{
		iov[0].iov_base = malloc(aadlen);
		iov[0].iov_len = aadlen;
		iov[1].iov_base = (void *)buf;
		iov[1].iov_len = out_msg_len;
		msg.msg_iovlen = 2;

		msg.msg_control = NULL;
		msg.msg_controllen = 0;

		// receive message
		recvmsg(opfd, &msg, MSG_DONTWAIT);

		free(iov[0].iov_base);
	}
	else
	{
		iov[0].iov_base = (void *)buf;
		iov[0].iov_len = out_msg_len;
		msg.msg_iovlen = 1;

		msg.msg_control = NULL;
		msg.msg_controllen = 0;

		// receive message
		recvmsg(opfd, &msg, MSG_DONTWAIT);
    }

	memcpy(out, buf, (out_msg_len) * sizeof(unsigned char));

	free(buf);
   free(cbuf);
	close(opfd);
	close(tfmfd);

   return 0;
}

static  size_t  get_size(char *name)
{
   size_t ret = -1;
   FILE *fp = fopen(name, "rb");
   if (!fp) {
      perror("fopen");
      return EXIT_FAILURE;
   }
   ret = fseek(fp, 0, SEEK_END);
   ret = ftell(fp);
   fclose(fp);
   return ret;
}
//#define TMP_FILE_FACE_DB_SAVE_GZIPDATA  "/tmp/face_db_org_data.bin.gz"
//#define TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT   "/tmp/ver_uh_face_db_org_data.bin.gz"
static int gen_uboot_head_and_ver()
{
   int ret;
   char cmd[128];
   image_header_t uh;
   unsigned int ver=0;

   //gzip压缩下
   sprintf(cmd,"cd /tmp;gzip -k -f %s", TMP_FILE_FACE_DB_SAVE_ORGDATA);
   ER(system(cmd));

   //填充uboot头
   memset((void*)&uh,0 ,sizeof(uh));
   uh.ih_magic = htonl(IH_MAGIC);
   strncpy((char*)uh.ih_name, "face_db", IH_NMLEN);
   uh.ih_load=htonl(CONFIG_MEM_FACE_DATA_BASE);
   uh.ih_comp=1;//gzip
   uh.ih_size = htonl(get_size(TMP_FILE_FACE_DB_SAVE_GZIPDATA));

   //生成 ver+uboothead+gzipdata 文件
   ER(save_buff_2_file((char*)&uh,sizeof(uh), TMP_FILE_FACE_DB_SAVE_UH));
   ER(save_buff_2_file((char*)&ver,sizeof(ver), TMP_FILE_FACE_DB_SAVE_VER));
   sprintf(cmd, "cd /tmp;cat %s %s %s >%s;", 
            TMP_FILE_FACE_DB_SAVE_VER,TMP_FILE_FACE_DB_SAVE_UH,
            TMP_FILE_FACE_DB_SAVE_GZIPDATA, TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT );
   ret = system(cmd);
   return ret;
}


static int calc_sha256(char *file, unsigned char *sha256)
{
   char cmd[128];
   int i=0;
   //使用命令
   sprintf(cmd, "sha256sum %s >%s ",file, TMP_FILE_FACE_DB_SAVE_SHA256);
   //printf("cmd=%s \n", cmd);
   ER(system(cmd));


   FILE *fp = fopen(TMP_FILE_FACE_DB_SAVE_SHA256, "rb");
   if (!fp) {
      perror("fopen");
      return EXIT_FAILURE;
   }

   for(i=0;i<32;i++){
      fscanf(fp,"%02hhx", (sha256+i));
      //printf("i=%d %02hhx\n",i, *(sha256+i));
   }

   fclose(fp);
   return 0;
}
//#define TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT   "/tmp/ver_uh_face_db_org_data.bin.gz"
//#define TMP_FILE_FACE_DB_SAVE_FWh_VER_UH_GZIPDAT  "/tmp/face_db_fwh_ver_uh_gzipdata"
static int add_k230_fimware_head()
{
   char cmd[128];
   firmware_head_s fh;
   uint32_t tmp_len;

   memset(&fh,0,sizeof(fh));
   
   fh.magic=MAGIC_NUM;
   tmp_len=get_size(TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT);
   fh.crypto_type=GCM_ONLY;
   uint8_t tmp_in[tmp_len];
   uint8_t out[tmp_len + 16];
   // copy plaintext to file
   ER(read_file_2_buff((char*)tmp_in, tmp_len, TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT));
   // encryption
   ER(cal_aes_gcm(aes_gcm.key,
                  aes_gcm.keybits,
                  ALG_OP_ENCRYPT,
                  tmp_in,
                  tmp_len,
                  aes_gcm.aad,
                  aes_gcm.aadlen,
                  aes_gcm.iv,
                  aes_gcm.ivlen,
                  out));
   ER(save_buff_2_file(out, (tmp_len + 16), TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT_ENC));
   fh.length=get_size(TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT_ENC);
   ER(save_buff_2_file((char*)&fh,sizeof(fh), TMP_FILE_FACE_DB_SAVE_FWH));
   sprintf(cmd, "cat %s %s > %s", 
         TMP_FILE_FACE_DB_SAVE_FWH,TMP_FILE_FACE_DB_SAVE_VER_UH_GZIPDAT_ENC,TMP_FILE_FACE_DB_SAVE_FWh_VER_UH_GZIPDAT);
         
   return system(cmd);
}
//#define TMP_FILE_FACE_DB_SAVE_FWh_VER_UH_GZIPDAT  "/tmp/face_db_fwh_ver_uh_gzipdata"
static int write_file_2_mtd(char *file)
{
   int ret = 0;
   char cmd[256];
   sprintf(cmd, "mtd=$(cat /proc/mtd  | grep face | cut -d ':' -f1);"
               "flashcp -v %s /dev/${mtd}; sync;",file );
   // printf("cmd=%s\n", cmd);
   ret = system(cmd);
   sprintf(cmd, "cd /tmp; rm -rf face_db_*");
   //system(cmd);
   return ret;

}
//fwh+ver+uh+gzipdata;
int save_face_db(char *buff, int len)
{
   
   ER(save_buff_2_file(buff,len,TMP_FILE_FACE_DB_SAVE_ORGDATA));
   ER(gen_uboot_head_and_ver());   
   ER(add_k230_fimware_head());   
   ER(write_file_2_mtd(TMP_FILE_FACE_DB_SAVE_FWh_VER_UH_GZIPDAT));     
   return 0;
}

int feature_db_save(uint32_t phyaddr, uint32_t length)
{
    int fd;
    int ret;
    void *vaddr;

    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        perror("open /dev/mem error\n");
        return -1;
    }
    vaddr = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, phyaddr);
    ret = save_face_db(vaddr, length);
    munmap(vaddr, length);
    fsync(fd);
    close(fd);

    return ret;
}
