#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tiny_sha1.h"
#include "zlib.h"

#define CHUNK       4096
#define THREAD_NUM  10
#define TXT_LINES   10000
#define LOOPS_TIMES 10

typedef struct
{
    int           id;
    char          path[64];
    char          path_zip[64];
    uint8_t       sha1[20];
    uint8_t       sha1_bak[20];
    pthread_t     thread;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
} stress_mem_context;

static stress_mem_context contexts[THREAD_NUM];

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level, unsigned char *in, unsigned char *out)
{
    int      ret, flush;
    unsigned have;
    z_stream strm;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;
    ret         = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush        = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out  = out;
            ret            = deflate(&strm, flush); /* no bad return value */
            usleep(1000);
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0); /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END); /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE *source, FILE *dest, unsigned char *in, unsigned char *out)
{
    int      ret;
    unsigned have;
    z_stream strm;

    /* allocate inflate state */
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;
    ret           = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out  = out;
            ret            = inflate(&strm, Z_NO_FLUSH);
            usleep(1000);
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT: ret = Z_DATA_ERROR; /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR: (void)inflateEnd(&strm); return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
                fputs("error writing stdout\n", stderr);
            break;
        case Z_STREAM_ERROR: fputs("invalid compression level\n", stderr); break;
        case Z_DATA_ERROR: fputs("invalid or incomplete deflate data\n", stderr); break;
        case Z_MEM_ERROR: fputs("out of memory\n", stderr); break;
        case Z_VERSION_ERROR: fputs("zlib version mismatch!\n", stderr);
    }
}

void wirte_rand(FILE *fd, int times)
{
    char string[64] = {0};
    for (int i = 0; i < times; i++) {
        int a   = rand();
        int len = snprintf(string, sizeof(string), "%d\n", a);
        fwrite(string, len, 1, fd);
    }
}

void creat_txt()
{
    FILE *fd = NULL;
    for (int i = 0; i < THREAD_NUM; i++) {
        snprintf(contexts[i].path, sizeof(contexts[i].path), "/dev/shm/%d.txt", i);
        snprintf(contexts[i].path_zip, sizeof(contexts[i].path_zip), "/dev/shm/%d.zip", i);
        fd = fopen(contexts[i].path, "w");
        if (fd == NULL) {
            printf("[stress_mem] file: open the output file : %s error!\n", contexts[i].path);
            exit(1);
        }
        wirte_rand(fd, TXT_LINES);
        fclose(fd);
    }
}

void get_sha1()
{
    printf("***************************************** get sha1 *************************************************\n");
    for (int i = 0; i < THREAD_NUM; i++) {
        FILE *fd = fopen(contexts[i].path, "rb");
        if (fd == NULL) {
            printf("[stress_mem] file: \"%s\" is not exist!!!\n", contexts[i].path);
            memset(contexts[i].sha1, 0, sizeof(contexts[i].sha1));
            exit(1);
        }

        tiny_sha1_context ctx;
        uint8_t          *data;
        int               bytes;

        data = malloc(1024);
        tiny_sha1_starts(&ctx);

        while ((bytes = fread(data, 1, 1024, fd)) != 0) {
            tiny_sha1_update(&ctx, (uint8_t *)data, bytes);
        }
        fclose(fd);
        free(data);

        tiny_sha1_finish(&ctx, contexts[i].sha1);

        printf(
            "[stress_mem] %s sha1: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
            contexts[i].path, contexts[i].sha1[0], contexts[i].sha1[1], contexts[i].sha1[2], contexts[i].sha1[3],
            contexts[i].sha1[4], contexts[i].sha1[5], contexts[i].sha1[6], contexts[i].sha1[7], contexts[i].sha1[8],
            contexts[i].sha1[9], contexts[i].sha1[10], contexts[i].sha1[11], contexts[i].sha1[12], contexts[i].sha1[13],
            contexts[i].sha1[14], contexts[i].sha1[15], contexts[i].sha1[16], contexts[i].sha1[17],
            contexts[i].sha1[18], contexts[i].sha1[19]);
    }
    printf("***************************************** get sha1 ************************************************\n\n\n");
}

void compress_zip(int index)
{
    FILE *fd_in  = NULL;
    FILE *fd_out = NULL;

    fd_in = fopen(contexts[index].path, "rb");
    if (fd_in == NULL) {
        printf("[compress_zip] file : open the input file : %s error!\n", contexts[index].path);
        goto _exit;
    }

    fd_out = fopen(contexts[index].path_zip, "wb");
    if (fd_out == NULL) {
        printf("[compress_zip] file : open the output file : %s error!\n", contexts[index].path_zip);
        goto _exit;
    }

    int ret = def(fd_in, fd_out, Z_BEST_COMPRESSION, contexts[index].in, contexts[index].out);
    if (ret != Z_OK) {
        zerr(ret);
        printf("[compress_zip] zip: compress file \"%s\" error!\n", contexts[index].path);
        goto _exit;
    }

_exit:
    if (fd_in != NULL) {
        fclose(fd_in);
    }

    if (fd_out != NULL) {
        fclose(fd_out);
    }
}

void decompress_zip(int index)
{
    FILE *fd_in  = NULL;
    FILE *fd_out = NULL;

    fd_in = fopen(contexts[index].path_zip, "rb");
    if (fd_in == NULL) {
        printf("[decompress_zip] file : open the input file : %s error!\n", contexts[index].path_zip);
        goto _exit;
    }

    fd_out = fopen(contexts[index].path, "wb");
    if (fd_out == NULL) {
        printf("[decompress_zip] file : open the output file : %s error!\n", contexts[index].path);
        goto _exit;
    }

    int ret = inf(fd_in, fd_out, contexts[index].in, contexts[index].out);
    if (ret != Z_OK) {
        zerr(ret);
        printf("[decompress_zip] zip: decompress file \"%s\" error!\n", contexts[index].path_zip);
        goto _exit;
    }

_exit:
    if (fd_in != NULL) {
        fclose(fd_in);
    }

    if (fd_out != NULL) {
        fclose(fd_out);
    }
}

void *thread_func(void *arg)
{
    int index = *(int *)arg;
    for (int i = 0; i < LOOPS_TIMES; i++) {
        printf("[thread]: thread %d start compress   %d times\n", index, i);
        compress_zip(index);
        printf("[thread]: thread %d end   compress   %d times\n", index, i);
        usleep(1000);
        printf("[thread]: thread %d start decompress %d times\n", index, i);
        decompress_zip(index);
        printf("[thread]: thread %d end   decompress %d times\n", index, i);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    creat_txt();
    get_sha1();

    for (int i = 0; i < THREAD_NUM; i++) {
        memcpy(contexts[i].sha1_bak, contexts[i].sha1, sizeof(contexts[i].sha1_bak));
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        contexts[i].id = i;
        if (pthread_create(&contexts[i].thread, NULL, thread_func, &contexts[i].id) != 0) {
            printf("[stress_mem] thread: create thread \"%d\" fail!!!\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        if (pthread_join(contexts[i].thread, NULL) != 0) {
            printf("[stress_mem] thread: join thread \"%d\" fail!!!\n", i);
            exit(1);
        }
    }
    get_sha1();

    for (int i = 0; i < THREAD_NUM; i++) {
        int   result     = memcmp(contexts[i].sha1_bak, contexts[i].sha1, sizeof(contexts[i].sha1));
        char *result_str = NULL;
        if (result == 0) {
            result_str = "success";
        }
        else {
            result_str = "fail";
        }
        printf("[compare] %s old <=> new: "
               "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x <=> "
               "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x [%s]\n",
               contexts[i].path, contexts[i].sha1_bak[0], contexts[i].sha1_bak[1], contexts[i].sha1_bak[2],
               contexts[i].sha1_bak[3], contexts[i].sha1_bak[4], contexts[i].sha1_bak[5], contexts[i].sha1_bak[6],
               contexts[i].sha1_bak[7], contexts[i].sha1_bak[8], contexts[i].sha1_bak[9], contexts[i].sha1_bak[10],
               contexts[i].sha1_bak[11], contexts[i].sha1_bak[12], contexts[i].sha1_bak[13], contexts[i].sha1_bak[14],
               contexts[i].sha1_bak[15], contexts[i].sha1_bak[16], contexts[i].sha1_bak[17], contexts[i].sha1_bak[18],
               contexts[i].sha1_bak[19], contexts[i].sha1[0], contexts[i].sha1[1], contexts[i].sha1[2],
               contexts[i].sha1[3], contexts[i].sha1[4], contexts[i].sha1[5], contexts[i].sha1[6], contexts[i].sha1[7],
               contexts[i].sha1[8], contexts[i].sha1[9], contexts[i].sha1[10], contexts[i].sha1[11],
               contexts[i].sha1[12], contexts[i].sha1[13], contexts[i].sha1[14], contexts[i].sha1[15],
               contexts[i].sha1[16], contexts[i].sha1[17], contexts[i].sha1[18], contexts[i].sha1[19], result_str);
    }

    return 0;
}
