#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite_util.h"
#include "png.h"

int vg_lite_load_png(vg_lite_buffer_t * buffer, const char * name)
{
    // Set status.
    int status = 0;
    png_image image;

    for (;;) {
        // Zero the t2D buffer structure.
        memset(buffer, 0, sizeof(vg_lite_buffer_t));

        // Construct the PNG image structure.
        memset(&image, 0, (sizeof image));
        image.version = PNG_IMAGE_VERSION;

        // Read the PNG header.
        if (!png_image_begin_read_from_file(&image, name))
            break;

        // Convert PNG format into t2D format.
        if (image.format == PNG_FORMAT_GRAY) {
            // Set to L8 format.
            buffer->format = VG_LITE_L8;
        } else {
            // Set to ARGB8888 format.
            buffer->format = VG_LITE_RGBA8888;
            image.format = PNG_FORMAT_RGBA;
        }

        // Set VGLite buffer width and height.
        buffer->width  = image.width;
        buffer->height = image.height;

        // Allocate the VGLite buffer memory.
        if (vg_lite_allocate(buffer) != 0)
            break;

        // Read the PNG image.
        if (!png_image_finish_read(&image, NULL, buffer->memory, buffer->stride, NULL))
            break;

        // Success.
        status = 1;
        break;
    }

    if (!status && (buffer->handle != NULL)) {
        // Free the VGLite buffer memory.
        vg_lite_free(buffer);
    }

    // Return the status.
    return status;
}
