#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include "zlib.h"
#include "gunzip.h"
#include <stdlib.h>

#define HEADER0			'\x1f'
#define HEADER1			'\x8b'
#define	ZALLOC_ALIGNMENT	16
#define HEAD_CRC		2
#define EXTRA_FIELD		4
#define ORIG_NAME		8
#define COMMENT			0x10
#define RESERVED		0xe0
#define DEFLATED		8

void *gzalloc(void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

void gzfree(void *x, void *addr, unsigned nb)
{
	free (addr);
}

int gzip_parse_header(const unsigned char *src, unsigned long len)
{
	int i, flags;

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != DEFLATED || (flags & RESERVED) != 0) {
		printf ("Error: Bad gzipped data\n");
		return (-1);
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= len) {
		printf ("Error: gunzip out of data in header\n");
		return (-1);
	}
	return i;
}

int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
						int stoponerr, int offset)
{
	z_stream s;
	int err = 0;
	int r;

	s.zalloc = gzalloc;
	s.zfree = gzfree;

	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf("Error: inflateInit2() returned %d\n", r);
		return -1;
	}
	s.next_in = src + offset;
	s.avail_in = *lenp - offset;
	s.next_out = dst;
	s.avail_out = dstlen;
	do {
		r = inflate(&s, Z_FINISH);
		if (stoponerr == 1 && r != Z_STREAM_END &&
			(s.avail_in == 0 || s.avail_out == 0 || r != Z_BUF_ERROR)) {
			printf("Error: inflate() returned %d\n", r);
			err = -1;
			break;
		}
	} while (r == Z_BUF_ERROR);
	*lenp = s.next_out - (unsigned char *) dst;
	inflateEnd(&s);

	return err;
}

int stand_gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp)
{
    int temp = 0;
    temp = *(int *)(src + (*lenp)-4);
    if(temp > dstlen){
        printf("dest len error %x %x \n",temp, dstlen);
        return  -4;
    }
    int offset = gzip_parse_header(src, *lenp);

	if (offset < 0)
		return offset;

	return zunzip(dst, dstlen, src, lenp, 1, offset);
}