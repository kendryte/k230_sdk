#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#if !defined(_WIN32)
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#endif

#include "vg_lite_util.h"
#include "vg_lite.h"

#ifdef HAVE_FBDEV
#include <linux/fb.h>

static int s_file;
static vg_lite_buffer_t s_framebuffer;
#endif

vg_lite_buffer_t * vg_lite_fb_open(void)
{
#ifdef HAVE_FBDEV
    struct fb_fix_screeninfo fix_info;
    struct fb_var_screeninfo var_info;

    // Open /dev/fb0 device.
    s_file = open("/dev/fb0", O_RDWR);
    if (s_file == -1) {
        return NULL;
    }

    // Get the fixed fbdev info.
    if (ioctl(s_file, FBIOGET_FSCREENINFO, &fix_info) < 0) {
        close(s_file);
        return NULL;
    }

    // Get the variable fbdev info.
    if (ioctl(s_file, FBIOGET_VSCREENINFO, &var_info) < 0) {
        close(s_file);
        return NULL;
    }

    switch (var_info.bits_per_pixel) {
        case 16:
            s_framebuffer.format = var_info.red.offset ? VG_LITE_BGR565 : VG_LITE_RGB565;
            break;

        case 32:
            s_framebuffer.format = var_info.red.offset ? VG_LITE_BGRA8888 : VG_LITE_RGBA8888;
            break;

        default:
            close(s_file);
            return NULL;
    }

    // Fill in framebuffer info.
    s_framebuffer.width = var_info.xres;
    s_framebuffer.height = var_info.yres;
    s_framebuffer.stride = fix_info.line_length;
    s_framebuffer.address = fix_info.smem_start;

    vg_lite_map_flag_t flag = VG_LITE_MAP_USER_MEMORY;
    int32_t fd = -1;
    if (vg_lite_map(&s_framebuffer, flag, fd) != VG_LITE_SUCCESS) {
        close(s_file);
        return NULL;
    }

    s_framebuffer.memory = mmap(NULL, s_framebuffer.stride * s_framebuffer.height, PROT_READ | PROT_WRITE, MAP_SHARED,
                                s_file, 0);
    if (s_framebuffer.memory == NULL) {
        vg_lite_unmap(&s_framebuffer);
        close(s_file);
    }

    return &s_framebuffer;
#else
    return NULL;
#endif
}

void vg_lite_fb_close(vg_lite_buffer_t * buffer)
{
#ifdef HAVE_FBDEV
    munmap(buffer->memory, buffer->stride * buffer->height);
    vg_lite_unmap(buffer);
    close(s_file);
#endif
}
