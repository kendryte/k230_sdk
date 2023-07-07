
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
#define __func__ __FUNCTION__
char *error_type[] = 
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",      
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }
static int   fb_width = 320, fb_height = 480;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * sys_fb;   //system framebuffer object to show the rendering result.
static vg_lite_buffer_t * fb;
static int has_fb = 0;

static vg_lite_buffer_t image[4];

void cleanup(void)
{
    int32_t i;
    
    if (has_fb) {
        // Close the framebuffer.
        vg_lite_fb_close(sys_fb);
    }
    
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    for (i = 0; i < 4; i ++)
    {
        if (image[i].handle != NULL) {
            // Free the image memory.
            vg_lite_free(&image[i]);
        }
    }

    vg_lite_close();
}

void create_index1(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 16;
    uint8_t *p = (uint8_t*)buffer->memory;
    uint8_t values[2] = {0xff, 0x00};

    for (i = 0; i < buffer->height; i++)
    {
        memset(p, values[(i / block) % 2], buffer->stride);
        p += buffer->stride;
    }
}
void create_index2(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 16;
    uint8_t *p = (uint8_t*)buffer->memory;
    uint8_t values[] = {0x00, 0x55, 0xaa, 0xff};
    
    for (i = 0; i < buffer->height; i++)
    {
        memset(p, values[(i / block) % 4], buffer->stride);
        p += buffer->stride;
    }
}
void create_index4(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 16;
    uint8_t *p = (uint8_t*)buffer->memory;
    uint8_t values[] = {0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb,
    0xcc, 0xdd, 0xee, 0xff
    };
    
    for (i = 0; i < buffer->height; i++)
    {
        memset(p, values[(i / block) % 16], buffer->stride);
        p += buffer->stride;
    }
}

void create_index8(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 1;
    uint8_t *p = (uint8_t*)buffer->memory;
   
    for (i = 0; i < buffer->height; i++)
    {
        memset(p, (i / block) % 256, buffer->stride);
        p += buffer->stride;
    }
}

void create_index_image(vg_lite_buffer_t *buffer)
{
    switch (buffer->format) {
        case VG_LITE_INDEX_8:
            create_index8(buffer);
            break;
            
        case VG_LITE_INDEX_4:
            create_index4(buffer);
            break;
            
        case VG_LITE_INDEX_2:
            create_index2(buffer);
            break;
            
        case VG_LITE_INDEX_1:
            create_index1(buffer);
            break;
            
        default:
            break;
    }
}

void create_index_table(uint32_t colors[256])
{
    int32_t i = 0;
    
    colors[0] = 0xff000000;
    colors[1] = 0xffffffff;
    colors[2] = 0xffff0000;
    colors[3] = 0xff00ff00;
    colors[4] = 0xff0000ff;
    colors[5] = 0xffffff00;
    colors[6] = 0xffff00ff;
    colors[7] = 0xff00ffff;
    colors[15] = 0xff000000;
    colors[14] = 0xffffffff;
    colors[13] = 0xffff0000;
    colors[12] = 0xff00ff00;
    colors[11] = 0xff0000ff;
    colors[10] = 0xffffff00;
    colors[9] = 0xffff00ff;
    colors[8] = 0xff00ffff;
    
    for (i = 16; i < 256; i++)
    {
        colors[i] = colors[i % 16];
    }
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    uint32_t i;
    uint32_t colors[256] = {0};
    vg_lite_matrix_t matrix;
    const char *names[4] = {"imgIndex1.png", "imgIndex2.png", "imgIndex4.png", "imgIndex8.png"};
    
    // Initialize vg_lite engine.
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(0, 0));
    
    filter = VG_LITE_FILTER_BI_LINEAR;

    for (i = 0; i < 4; i++)
    {
        image[i].format = VG_LITE_INDEX_1 + i;
        image[i].width = 256;
        image[i].height = 256;
        CHECK_ERROR(vg_lite_allocate(&image[i]));
        create_index_image(&image[i]);
    }
    create_index_table(colors);
    
    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);
    
    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Build a 33 degree rotation matrix from the center of the buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 2.0f, fb_height / 2.0f, &matrix);
    vg_lite_rotate(33.0f, &matrix);
    vg_lite_translate(fb_width / -2.0f, fb_height / -2.0f, &matrix);
    vg_lite_scale((vg_lite_float_t) fb_width / (vg_lite_float_t) image[0].width,
                  (vg_lite_float_t) fb_height / (vg_lite_float_t) image[0].height, &matrix);
    CHECK_ERROR(vg_lite_set_CLUT(2, colors));
    CHECK_ERROR(vg_lite_set_CLUT(4, colors));
    CHECK_ERROR(vg_lite_set_CLUT(16, colors));
    CHECK_ERROR(vg_lite_set_CLUT(256, colors));
    for (i = 0; i < 4; i++)
    {
        // Clear the buffer with blue.
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFaabbcc));
        // Blit the image using the matrix.
        CHECK_ERROR(vg_lite_blit(fb, &image[i], &matrix, VG_LITE_BLEND_NONE, 0, filter));
        CHECK_ERROR(vg_lite_finish());
        // Save PNG file.
        vg_lite_save_png(names[i], fb);
    }

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
