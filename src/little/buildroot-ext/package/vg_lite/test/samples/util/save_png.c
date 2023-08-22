#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite_util.h"
#include "png.h"

int vg_lite_save_png(const char *name, vg_lite_buffer_t *buffer)
{
    uint8_t *memory, *p, *q;
    int x, y;
    png_image image;
    int status;
    uint16_t color;

    if (buffer->format == VG_LITE_L8) {
        /* Construct the PNG image structure. */
        png_image image;
        memset(&image, 0, (sizeof image));

        image.version = PNG_IMAGE_VERSION;
        image.width   = buffer->width;
        image.height  = buffer->height;
        image.format  = (  buffer->format == VG_LITE_L8 ? PNG_FORMAT_GRAY
                         : buffer->format == VG_LITE_RGBA8888 ? PNG_FORMAT_ARGB
                         : PNG_FORMAT_ABGR);

        /* Write the PNG image. */
        return png_image_write_to_file(&image, name, 0, buffer->memory, buffer->stride, NULL);
    }

    /* Allocate RGB memory buffer. */
    memory = malloc(buffer->width * buffer->height * 3);
    if (memory == NULL) {
        return 0;
    }

    for (y = 0; y < buffer->height; y++) {
        p = (uint8_t*) buffer->memory + y * buffer->stride;
        q = memory + y * buffer->width * 3;
        for (x = 0; x < buffer->width; x++, q += 3) {
            switch (buffer->format) {
#if defined(GPU_CHIP_ID_GCNanoUltraV)
                case VG_LITE_RGBA5658_PLANAR:
                case VG_LITE_ARGB8565_PLANAR:
#endif
                case VG_LITE_RGB565:
                    color = *(uint16_t *)p;
                    p += 2;
                    q[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    q[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                    q[2] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                    break;

#if defined(GPU_CHIP_ID_GCNanoUltraV)
                case VG_LITE_ABGR8565_PLANAR:
                case VG_LITE_BGRA5658_PLANAR:
#endif
                case VG_LITE_BGR565:
                    color = *(uint16_t *)p;
                    p += 2;
                    q[0] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                    q[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                    q[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    break;

#if defined(GPU_CHIP_ID_GCNanoUltraV)
                case VG_LITE_ABGR8565:
                    p += 1;
                    color = *(uint8_t *)p | *(uint8_t *)(p + 1) << 8;
                    p += 2;
                    q[0] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                    q[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                    q[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    break;

                case VG_LITE_BGRA5658:
                    color = *(uint8_t *)p | *(uint8_t *)(p + 1) << 8;
                    p += 3;
                    q[0] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                    q[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                    q[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    break;

                case VG_LITE_ARGB8565:
                    p += 1;
                    color = *(uint8_t *)p | *(uint8_t *)(p + 1) << 8;
                    p += 2;
                    q[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    q[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                    q[2] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                    break;

                case VG_LITE_RGBA5658:
                    color = *(uint8_t *)p | *(uint8_t *)(p + 1) << 8;
                    p += 3;
                    q[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    q[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                    q[2] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                    break;

                case VG_LITE_RGB888:
                    q[0] = p[0];
                    q[1] = p[1];
                    q[2] = p[2];
                    p += 3;
                    break;

                case VG_LITE_BGR888:
                    q[0] = p[2];
                    q[1] = p[1];
                    q[2] = p[0];
                    p += 3;
                    break;
#endif

                case VG_LITE_RGBA8888:
                case VG_LITE_RGBX8888:
                    q[0] = p[0];
                    q[1] = p[1];
                    q[2] = p[2];
                    p += 4;
                    break;

                case VG_LITE_ARGB8888:
                case VG_LITE_XRGB8888:
                    q[0] = p[1];
                    q[1] = p[2];
                    q[2] = p[3];
                    p += 4;
                    break;

                case VG_LITE_BGRA8888:
                case VG_LITE_BGRX8888:
                    q[0] = p[2];
                    q[1] = p[1];
                    q[2] = p[0];
                    p += 4;
                    break;

                case VG_LITE_RGBA4444:
                    color = *(uint16_t*)p;
                    p += 2;
                    q[0] = (color & 0x000F) << 4;
                    q[1] = (color & 0x00F0);
                    q[2] = (color & 0x0F00) >> 4;
                    break;

                case VG_LITE_BGRA4444:
                    color = *(uint16_t*)p;
                    p += 2;
                    q[2] = (color & 0x000F) << 4;
                    q[1] = (color & 0x00F0);
                    q[0] = (color & 0x0F00) >> 4;
                    break;

                case VG_LITE_ABGR4444:
                    color = *(uint16_t*)p;
                    color = (color >> 4);
                    p += 2;
                    q[2] = (color & 0x000F) << 4;
                    q[1] = (color & 0x00F0);
                    q[0] = (color & 0x0F00) >> 4;
                    break;

                case VG_LITE_ARGB4444:
                    color = *(uint16_t*)p;
                    color = (color >> 4);
                    p += 2;
                    q[0] = (color & 0x000F) << 4;
                    q[1] = (color & 0x00F0);
                    q[2] = (color & 0x0F00) >> 4;
                    break;

                case VG_LITE_BGRA5551:
                    color = *(uint16_t*)p;
                    p += 2;
                    q[0] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);
                    q[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                    q[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    break;

                case VG_LITE_ABGR1555:
                    color = *(uint16_t*)p;
                    color = (color >> 1);
                    p += 2;
                    q[0] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);
                    q[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                    q[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    break;

                case VG_LITE_RGBA5551:
                    color = *(uint16_t*)p;
                    p += 2;
                    q[2] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);
                    q[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                    q[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    break;

                case VG_LITE_ARGB1555:
                    color = *(uint16_t*)p;
                    color = (color >> 1);
                    p += 2;
                    q[2] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);
                    q[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                    q[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                    break;

                case VG_LITE_RGBA2222:
                    color = *(uint8_t*)p;
                    p += 1;
                    q[0] = (color & 0x03) << 2 | (color & 0x03) << 4 | (color & 0x03) << 6 |(color & 0x03);
                    q[1] = (color & 0x0C) << 2 | (color & 0x0C) << 4 | (color & 0x0C) << 6 |(color & 0x0C);
                    q[2] = (color & 0x30) << 2 | (color & 0x30) << 4 | (color & 0x30) << 6 |(color & 0x30);
                    break;
                    
                case VG_LITE_BGRA2222:
                    color = *(uint8_t*)p;
                    p += 1;
                    q[2] = (color & 0x03) << 2 | (color & 0x03) << 4 | (color & 0x03) << 6 |(color & 0x03);
                    q[1] = (color & 0x0C) << 2 | (color & 0x0C) << 4 | (color & 0x0C) << 6 |(color & 0x0C);
                    q[0] = (color & 0x30) << 2 | (color & 0x30) << 4 | (color & 0x30) << 6 |(color & 0x30);
                    break;

                case VG_LITE_ARGB2222:
                    color = *(uint8_t*)p;
                    p += 1;
                    q[0] = (color & 0x0C) << 2 | (color & 0x0C) << 4 | (color & 0x0C) << 6 |(color & 0x0C);
                    q[1] = (color & 0x30) << 2 | (color & 0x30) << 4 | (color & 0x30) << 6 |(color & 0x30);
                    q[2] = (color & 0xC0) << 2 | (color & 0xC0) << 4 | (color & 0xC0) << 6 |(color & 0xC0);
                    break;

                case VG_LITE_ABGR2222:
                    color = *(uint8_t*)p;
                    p += 1;
                    q[2] = (color & 0x0C) << 2 | (color & 0x0C) << 4 | (color & 0x0C) << 6 |(color & 0x0C);
                    q[1] = (color & 0x30) << 2 | (color & 0x30) << 4 | (color & 0x30) << 6 |(color & 0x30);
                    q[0] = (color & 0xC0) << 2 | (color & 0xC0) << 4 | (color & 0xC0) << 6 |(color & 0xC0);
                    break;

                case VG_LITE_A8:
                case VG_LITE_L8:
                    q[0] = q[1] = q[2] = p[0];
                    p++;
                    break;

                case VG_LITE_YUYV:
                    /* YUYV not supported yet. */

                default:
                    break;
            }
        }
    }

    /* Construct the PNG image structure. */
    memset(&image, 0, (sizeof image));

    image.version = PNG_IMAGE_VERSION;
    image.width   = buffer->width;
    image.height  = buffer->height;
    image.format  = PNG_FORMAT_RGB;

    /* Write the PNG image. */
    status = png_image_write_to_file(&image, name, 0, memory, buffer->width * 3, NULL);

    /* Free the RGB memory buffer.*/
    free(memory);

    /* Success. */
    return status;
}
