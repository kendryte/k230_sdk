#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int g_memmap_fd = -1;
static int g_page_size = 0x1000;

static int sfs_memmap_init(void)
{
	int fd;
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		printf("open /dev/mem failed\n");
		return -1;
	}
	g_page_size = getpagesize();
	g_memmap_fd = fd;
	return 0;
}

void *sfs_mmap(unsigned long phys, unsigned long size,
		unsigned long *mapped_addr, unsigned long *mapped_size)
{
	void *page_addr;
	unsigned long page_align_phys = phys & ~(g_page_size - 1);
	unsigned long page_offset = phys - page_align_phys;
	unsigned long page_align_size = (size + g_page_size - 1) & ~(g_page_size - 1);
	if (g_memmap_fd < 0) {
		sfs_memmap_init();
	}

	page_addr = mmap(NULL, page_align_size, PROT_READ|PROT_WRITE,
			MAP_SHARED, g_memmap_fd, page_align_phys);
	if ((void *)page_addr == MAP_FAILED) {
		printf("mmap %p failed\n", page_align_phys);
		return NULL;
	}
	*mapped_addr = (unsigned long)page_addr;
	*mapped_size = page_align_size;
	return (void *)((unsigned long)page_addr + page_offset);
}

void sfs_munmap(unsigned long mapped_addr, unsigned long size)
{
	munmap((void *)mapped_addr, size);
}

