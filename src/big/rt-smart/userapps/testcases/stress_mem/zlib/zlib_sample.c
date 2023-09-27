/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2005 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.
 *
 * RT-Thread Development Team port
 *
 * Change Logs:
 * Date         Author        Notes
 * 2019-02-14   SummerGift    first version
 */

#include <rtconfig.h>
#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <dfs_posix.h>
#include "zlib.h"

#define CHUNK 4096
static char in[CHUNK];
static char out[CHUNK];

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

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
int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
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
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
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
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

int zlib_test(int argc, char ** argv)
{
    FILE *fd_in, *fd_out;
    int ret  = 0;

    if (argc != 4)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("zlib_test -c [file] [cmprs_file]          -compress \"file\" to \"cmprs_file\" \n");
        rt_kprintf("zlib_test -d [cmprs_file] [dcmprs_file]   -dcompress \"cmprs_file\" to \"dcmprs_file\" \n");

        ret = -1;
        goto _exit;
    }

    fd_in = fopen(argv[2], "r");
    if (fd_in == NULL)
    {
        rt_kprintf("[zlib] open the input file : %s error!\n", argv[2]);
        ret = -1;
        goto _exit;
    }

    fd_out = fopen(argv[3], "w+");
    if (fd_out == NULL)
    {
        rt_kprintf("[zlib] open the output file : %s error!\n", argv[3]);
        ret = -1;
        goto _exit;
    }

    if(memcmp("-c", argv[1], strlen(argv[1])) == 0)
    {
        ret = def(fd_in, fd_out, Z_BEST_COMPRESSION);
        if (ret != Z_OK)
        {
            zerr(ret);
            rt_kprintf("[zlib] compress file error!\n");
            return ret;
        }
    }
    else if(memcmp("-d", argv[1], strlen(argv[1])) == 0)
    {
        ret = inf(fd_in, fd_out);
        if (ret != Z_OK)
        {
            zerr(ret);
            rt_kprintf("[zlib] decompress file error!\n");
            return ret;
        }
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("zlib_test -c [file] [cmprs_file]          -compress \"file\" to \"cmprs_file\" \n");
        rt_kprintf("zlib_test -d [cmprs_file] [dcmprs_file]   -dcompress \"cmprs_file\" to \"dcmprs_file\" \n");

        ret = -1;
        goto _exit;
    }

_exit:
    if(fd_in != NULL)
    {
        fclose(fd_in);
    }

    if(fd_out != NULL)
    {
        fclose(fd_out);
    }

    return ret;
}

#ifdef RT_USING_FINSH
#ifdef FINSH_USING_MSH

#include <finsh.h>

MSH_CMD_EXPORT(zlib_test, zlib_test compress and decompress test);
#endif
#endif
