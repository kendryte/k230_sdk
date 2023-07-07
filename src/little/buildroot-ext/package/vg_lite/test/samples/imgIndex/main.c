
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
static int   fb_width = 320, fb_height = 480;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * sys_fb;   //system framebuffer object to show the rendering result.
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t raw;
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
    
    if (raw.handle != NULL) {
        // Free the raw memory.
        vg_lite_free(&raw);
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
    uint32_t feature_check = 0;
    vg_lite_filter_t filter;
    uint32_t i;
    uint32_t colors[256] = {0};
    vg_lite_matrix_t matrix;
#if DDRLESS
    const char *names[4] = {"imgIndex1_dl.png", "imgIndex2_dl.png", "imgIndex4_dl.png", "imgIndex8_dl.png"};
#else
    const char *names[4] = {"imgIndex1.png", "imgIndex2.png", "imgIndex4.png", "imgIndex8.png"};
#endif
    
    // Initialize vg_lite engine.
#if DDRLESS
    vg_lite_error_t error = vg_lite_init(fb_width, fb_height);
#else
    vg_lite_error_t error = vg_lite_init(0, 0);
#endif
    if (error) {
        printf("vg_lite engine init failed: vg_lite_blit_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_INDEX_FORMAT);
    if (feature_check == 0) {
        printf("image index is not supported.\n");
        return -1;
    }
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_FILTER);
    if (feature_check == 0) {
        filter = VG_LITE_FILTER_POINT;
    } else {
        filter = VG_LITE_FILTER_POINT;
    }

    for (i = 0; i < 4; i++)
    {
        image[i].format = VG_LITE_INDEX_1 + i;
        image[i].width = 256;
        image[i].height = 256;
        vg_lite_allocate(&image[i]);
        create_index_image(&image[i]);
    }
    create_index_table(colors);
    
    // Try to open the framebuffer.
    sys_fb = vg_lite_fb_open();
    has_fb = (sys_fb != NULL);
    if (argc == 2) {
        fb_width = atoi(argv[1]);
        if ((fb_width <= 0) && has_fb) {
            fb_width = sys_fb->width;
            fb_height = sys_fb->height;
        }
    }
    else
        if (argc == 3) {
            fb_width = atoi(argv[1]);
            fb_height = atoi(argv[2]);
        }
    
    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);
    
    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
#if DDRLESS
    buffer.format = VG_LITE_RGBA8888;
#else
	buffer.format = VG_LITE_RGBA8888;
#endif
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    // Build a 33 degree rotation matrix from the center of the buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 2.0f, fb_height / 2.0f, &matrix);
    vg_lite_rotate(33.0f, &matrix);
    vg_lite_translate(fb_width / -2.0f, fb_height / -2.0f, &matrix);
    vg_lite_scale((vg_lite_float_t) fb_width / (vg_lite_float_t) image[0].width,
                  (vg_lite_float_t) fb_height / (vg_lite_float_t) image[0].height, &matrix);
    vg_lite_set_CLUT(2, colors);
    vg_lite_set_CLUT(4, colors);
    vg_lite_set_CLUT(16, colors);
    vg_lite_set_CLUT(256, colors);
   // for (i = 0; i < 2; i++)
    //{
        // Clear the buffer with blue.RGBA
        //vg_lite_clear(fb, NULL, 0xffff0000);
        //vg_lite_finish();

        // Blit the image using the matrix.
        vg_lite_blit(fb, &image[2], &matrix, VG_LITE_BLEND_NONE, 0, filter);
        vg_lite_finish();

        // Save PNG file.
        vg_lite_save_png("imgIndex3.png", fb);
		vg_lite_save_raw("imgIndex3.raw", fb);
		vg_lite_save_png("image1.png", &image[2]);
		vg_lite_save_raw("image1.raw", &image[2]);
   // }
    
#if 0
    // Check the result with golden.
    if (!vg_lite_load_raw(&raw, golden)) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }
    
    if (memcmp((&raw)->memory, fb->memory, buffer.stride * buffer.height)) {
        printf("result error\n");
    } else {
        printf("result correct\n");
    }

    // Save RAW file.
    if (getenv("GOLDEN") != NULL) {
        if (!strcmp(getenv("GOLDEN"), "1")) {
            if(!vg_lite_save_raw(golden, &buffer))
            {
                printf("save raw file error\n");
                return -1;
            }
        }
    }
#endif
    
    // Show rendering on screen.
    if (has_fb) {
        vg_lite_matrix_t matrix;
        vg_lite_identity(&matrix);
        vg_lite_blit(sys_fb, fb, &matrix, VG_LITE_BLEND_NONE, 0, filter);
        vg_lite_finish();
    }

    // Cleanup.
    cleanup();
    return 0;
}
