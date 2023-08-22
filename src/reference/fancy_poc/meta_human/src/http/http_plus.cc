#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <assert.h>
#include    <unistd.h> 
#include    <sys/stat.h>
#include    "http.h"
#include    "base64.h"

char * http_post_plus(const char *url, const char *post_str, unsigned int post_length)
{
	char *encode_out = reinterpret_cast<char*>(malloc(BASE64_ENCODE_OUT_SIZE(post_length) + 1));
	unsigned int outlen = base64_encode(post_str, post_length, encode_out);    
    encode_out[outlen] = '\0';

    // for(unsigned int i=0; i<outlen; i++) {
	// 	printf("%c", encode_out[i]);
	// }

    char *resp = http_post(url, encode_out);
    free(encode_out);

    return resp;
}

#ifdef  DEBUG_HTTP_PLUS
int main(int argc, char *argv[])
{
#if 0
    //  ./a.out http://10.100.201.105:8080 1234567890qw
    //post(argv[1], argv[2], strlen(argv[2]));
#else
    unsigned char *buff;
    unsigned int len;

    FILE *fd = fopen("output_30_0.bin", "rb+" );
    if(fd == NULL) {
        printf("fopen fail\n");
        return -1;
    }
    printf("fopen ok\n");
    struct stat statbuf;

    int ret;

    ret = stat("output_30_0.bin", &statbuf);//调用stat函数
    if(ret != 0) return -1; //获取失败。

    len = statbuf.st_size;
    printf("file size = %d\n", len);

    buff = malloc(statbuf.st_size);
    fread(buff, statbuf.st_size, 1, fd);
    fclose(fd);

    post("http://10.100.201.105:8080", buff, len);
#endif    
    return 0;
}
#endif
