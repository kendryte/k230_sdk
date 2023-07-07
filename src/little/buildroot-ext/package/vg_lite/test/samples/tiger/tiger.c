#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "tiger_paths.h"

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
static int fb_width = 640, fb_height = 480;
static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

void cleanup(void)
{
    int32_t i;

    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    for (i = 0; i < pathCount; i++)
    {
        vg_lite_clear_path(&path[i]);
    }
    
    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    int i;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;

    /* Initialize vglite. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    // Setup a scale at center of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 2 - 20 * fb_width / 640.0f,
                      fb_height / 2 - 100 * fb_height / 480.0f, &matrix);
    vg_lite_scale(4, 4, &matrix);
    vg_lite_scale(fb_width / 640.0f, fb_height / 480.0f, &matrix);

    // Draw the path using the matrix.
    for (i = 0; i < pathCount; i++)
    {
        CHECK_ERROR(vg_lite_draw(fb, &path[i], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[i]));
    }
    CHECK_ERROR(vg_lite_finish());
    // Save PNG file.
    vg_lite_save_png("tiger.png", fb);

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
