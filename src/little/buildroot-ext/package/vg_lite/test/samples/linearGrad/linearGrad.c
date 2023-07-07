
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 480

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
static int   fb_width = DEFAULT_WIDTH, fb_height = DEFAULT_HEIGHT;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t image;
int frames = 1; //Frames to render
/*
*-----*
/       \
/         \
*           *
|          /
|         X
|          \
*           *
\         /
\       /
*-----*
*/
static char path_data[] = {
    2, 0, 0, // moveto   -5,-10
    4, 100, 0,  // lineto    5,-10
    4, 70, 50,  // lineto    5,-10
    4, 100, 100,  // lineto   10, -5
    4, 0, 100,    // lineto    0,  0
    4, 40, 80,  // lineto    5,-10
    4, 0, 0,   // lineto   10,  5
    0, // end
};

static vg_lite_path_t path = {
    { 0,   0, // left,top
      100, 100 }, // right,bottom
    VG_LITE_HIGH, // quality
    VG_LITE_S8, // -128 to 127 coordinate range
    {0}, // uploaded
    sizeof(path_data), // path length
    path_data, // path data
    1
};

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image);
    }

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_linear_gradient_t grad;
    uint32_t ramps[] = {0xff000000, 0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffffff};
    uint32_t stops[] = {0, 66, 122, 200, 255};
    vg_lite_matrix_t *matGrad;
    vg_lite_matrix_t matPath;
    int fcount = 0;

    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    printf("Render size: %d x %d\n", fb_width, fb_height);
    while (frames > 0)
    {
        memset(&grad, 0, sizeof(grad));
        if (VG_LITE_SUCCESS != vg_lite_init_grad(&grad)) {
            printf("Linear gradient is not supported.\n");
            vg_lite_close();
            return 0;
        }

        vg_lite_set_grad(&grad, 5, ramps, stops);
        vg_lite_update_grad(&grad);
        matGrad = vg_lite_get_grad_matrix(&grad);
        vg_lite_identity(matGrad);
        vg_lite_rotate(30.0f, matGrad);

        // Clear the buffer with blue.
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xffaabbcc));
        vg_lite_identity(&matPath);
        vg_lite_scale(4.0f, 4.0f, &matPath);

        // Fill the path using an image.
        CHECK_ERROR(vg_lite_draw_gradient(fb, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &grad, VG_LITE_BLEND_NONE));
        CHECK_ERROR(vg_lite_finish());
        vg_lite_clear_grad(&grad);
        frames--;
        printf("frame %d done\n", fcount++);
    }

    // Save PNG file.
    vg_lite_save_png("linearGrad.png", fb);

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
