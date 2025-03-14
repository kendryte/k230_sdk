/****************************************************************************
*
*    Copyright 2012 - 2022 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#include "vg_lite_context.h"

/* Global context variables and feature table.
*/
vg_lite_context_t   s_context = { 0 };
uint32_t            command_buffer_size = VG_LITE_COMMAND_BUFFER_SIZE;
uint32_t            submit_flag = 0;

/* Initialize the feature table of a chip. */
vg_lite_ftable_t    s_ftable = {
        gcFEATURE_VG_IM_INDEX_FORMAT,
        gcFEATURE_VG_SCISSOR,
        gcFEATURE_VG_BORDER_CULLING,
        gcFEATURE_VG_RGBA2_FORMAT,
        gcFEATURE_VG_QUALITY_8X,
        gcFEATURE_VG_IM_FASTCLEAR,
        gcFEATURE_VG_RADIAL_GRADIENT,
        gcFEATURE_VG_GLOBAL_ALPHA,
        gcFEATURE_VG_RGBA8_ETC2_EAC,
        gcFEATURE_VG_COLOR_KEY,
        gcFEATURE_VG_DOUBLE_IMAGE,
        gcFEATURE_VG_YUV_OUTPUT,
        gcFEATURE_VG_FLEXA,
        gcFEATURE_VG_24BIT,
        gcFEATURE_VG_DITHER,
        gcFEATURE_VG_USE_DST,
        gcFEATURE_VG_PE_CLEAR,
        gcFEATURE_VG_IM_INPUT,
        gcFEATURE_VG_DEC_COMPRESS,
        gcFEATURE_VG_LINEAR_GRADIENT_EXT,
        gcFEATURE_VG_MASK,
        gcFEATURE_VG_MIRROR,
        gcFEATURE_VG_GAMMA,
        gcFEATURE_VG_NEW_BLEND_MODE,
        gcFEATURE_VG_STENCIL,
        gcFEATURE_VG_SRC_PREMULTIPLIED,
        gcFEATURE_VG_HW_PREMULTIPLY,
        gcFEATURE_VG_COLOR_TRANSFORMATION,
        gcFEATURE_VG_LVGL_SUPPORT,
        gcFEATURE_VG_INDEX_ENDIAN,
        gcFEATURE_VG_24BIT_PLANAR,
        gcFEATURE_VG_PIXEL_MATRIX,
        gcFEATURE_VG_NEW_IMAGE_INDEX,
        gcFEATURE_VG_PARALLEL_PATHS,
        gcFEATURE_VG_STRIPE_MODE,
        gcFEATURE_VG_IM_DEC_INPUT,
        gcFEATURE_VG_GAUSSIAN_BLUR,
        gcFEATURE_VG_RECTANGLE_TILED_OUT,
        gcFEATURE_VG_TESSELLATION_TILED_OUT,
        gcFEATURE_VG_IM_REPEAT_REFLECT,
        gcFEATURE_VG_YUY2_INPUT,
        gcFEATURE_VG_YUV_INPUT,
        gcFEATURE_VG_YUV_TILED_INPUT,
        gcFEATURE_VG_AYUV_INPUT,
        gcFEATURE_VG_16PIXELS_ALIGNED,
};


static vg_lite_error_t check_hardware_chip_info(void)
{
    uint32_t chip_id = 0, chip_rev = 0, cid = 0;

    vg_lite_get_product_info(NULL, &chip_id, &chip_rev);
    vg_lite_get_register(0x30, &cid);

    if (CHIPID != chip_id || REVISION != chip_rev || CID != cid) {
        printf("VGLite API initialization Error!!! \nHardware ChipId: 0x%X  ChipRevision: 0x%X  Cid: 0x%X \n", chip_id, chip_rev, cid);
        printf("NOT match vg_lite_options.h CHIPID: 0x%X  REVISION: 0x%X  CID: 0x%X \n", CHIPID, REVISION, CID);
        return VG_LITE_NOT_SUPPORT;
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t check_compress(
    vg_lite_buffer_format_t format,
    vg_lite_compress_mode_t compress_mode,
    vg_lite_buffer_layout_t tiled,
    uint32_t width,
    uint32_t height
)
{
#if gcFEATURE_VG_DEC_COMPRESS
    vg_lite_error_t error = VG_LITE_SUCCESS;

    if (compress_mode) {
        if (compress_mode > VG_LITE_DEC_HV_SAMPLE || compress_mode < VG_LITE_DEC_DISABLE)
            return VG_LITE_INVALID_ARGUMENT;

        if (tiled) {
            if (width % 16 || height % 4)
                return VG_LITE_INVALID_ARGUMENT;
        }
        else {
            if (width % 16 || compress_mode == VG_LITE_DEC_HV_SAMPLE)
                return VG_LITE_INVALID_ARGUMENT;
        }

        if (format != VG_LITE_XBGR8888 && format != VG_LITE_XRGB8888 && format != VG_LITE_BGRX8888 && format != VG_LITE_RGBX8888 &&
            format != VG_LITE_ABGR8888 && format != VG_LITE_ARGB8888 && format != VG_LITE_BGRA8888 && format != VG_LITE_RGBA8888 &&
            format != VG_LITE_RGB888 && format != VG_LITE_BGR888)
            return VG_LITE_INVALID_ARGUMENT;
    }

    return error;
#else
    return VG_LITE_SUCCESS;
#endif
}

static vg_lite_float_t _calc_decnano_compress_ratio(
    vg_lite_buffer_format_t format,
    vg_lite_compress_mode_t compress_mode
)
{
    vg_lite_float_t ratio = 1.0f;

#if gcFEATURE_VG_DEC_COMPRESS
    switch (compress_mode) {
    case VG_LITE_DEC_NON_SAMPLE:
        switch (format) {
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBA8888:
            ratio = 0.625f;
            break;
        case VG_LITE_XBGR8888:
        case VG_LITE_XRGB8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_RGBX8888:
            ratio = 0.5f;
            break;
        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
            ratio = 0.667f;
            break;
        default:
            return ratio;
        }
        break;

    case VG_LITE_DEC_HSAMPLE:
        switch (format) {
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBA8888:
        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
            ratio = 0.5f;
            break;
        case VG_LITE_XBGR8888:
        case VG_LITE_XRGB8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_RGBX8888:
            ratio = 0.375f;
            break;
        default:
            return ratio;
        }
        break;

    case VG_LITE_DEC_HV_SAMPLE:
        switch (format) {
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBA8888:
            ratio = 0.375f;
            break;
        case VG_LITE_XBGR8888:
        case VG_LITE_XRGB8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_RGBX8888:
            ratio = 0.25f;
            break;
        default:
            return ratio;
        }
        break;
    default:
        return ratio;
    }
#endif

    return ratio;
}

static int32_t has_valid_command_buffer(vg_lite_context_t *context)
{
    if (context == NULL)
        return 0;
    if (context->command_buffer_current >= CMDBUF_COUNT)
        return 0;
    if (context->command_buffer == NULL)
        return 0;
    if (context->command_buffer[context->command_buffer_current] == NULL)
        return 0;

    return 1;
}

typedef vg_lite_float_t  FLOATVECTOR4[4];

static void ClampColor(FLOATVECTOR4 Source, FLOATVECTOR4 Target, uint8_t Premultiplied)
{
    vg_lite_float_t colorMax;
    /* Clamp the alpha channel. */
    Target[3] = CLAMP(Source[3], 0.0f, 1.0f);

    /* Determine the maximum value for the color channels. */
    colorMax = Premultiplied ? Target[3] : 1.0f;

    /* Clamp the color channels. */
    Target[0] = CLAMP(Source[0], 0.0f, colorMax);
    Target[1] = CLAMP(Source[1], 0.0f, colorMax);
    Target[2] = CLAMP(Source[2], 0.0f, colorMax);
}

static uint8_t PackColorComponent(vg_lite_float_t value)
{
    /* Compute the rounded normalized value. */
    vg_lite_float_t rounded = value * 255.0f + 0.5f;

    /* Get the integer part. */
    int32_t roundedInt = (int32_t)rounded;

    /* Clamp to 0..1 range. */
    uint8_t clamped = (uint8_t)CLAMP(roundedInt, 0, 255);

    /* Return result. */
    return clamped;
}

#if DUMP_IMAGE
static void dump_img(void * memory, int32_t width, int32_t height, vg_lite_buffer_format_t format)
{
    FILE * fp;
    char imgname[255] = {'\0'};
    static int32_t num = 1;
    uint32_t* pt = (uint32_t*) memory;
    int32_t i;

    sprintf(imgname, "img_pid%d_%d.txt", getpid(), num++);
    
    fp = fopen(imgname, "w");
    
    if (fp == NULL)
        printf("error!\n");
    

    switch (format) {
        case VG_LITE_INDEX_1:
            for(i = 0; i < width * height / 32; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_INDEX_2:
            for(i = 0; i < width * height / 16; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_INDEX_4:
            for(i = 0; i < width * height / 8; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_INDEX_8:
            for(i = 0; i < width * height / 4; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;

        case VG_LITE_RGBA2222:
            for(i = 0; i < width * height / 4; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;

        case VG_LITE_RGBA4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
            for(i = 0; i < width * height / 2; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        case VG_LITE_RGBA8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
            for(i = 0; i < width * height; ++i)
            {
                fprintf(fp, "0x%08x\n",pt[i]);
            }
            break;
            
        default:
            break;
    }
    fclose(fp);
    fp = NULL;
}
#endif

static uint32_t rgb_to_l(uint32_t color)
{
    uint32_t l = (uint32_t)((0.2126f * (vg_lite_float_t)(color & 0xFF)) +
                            (0.7152f * (vg_lite_float_t)((color >> 8) & 0xFF)) +
                            (0.0722f * (vg_lite_float_t)((color >> 16) & 0xFF)));
    return l | (l << 24);
}

/* Get the bpp information of a color format. */
void get_format_bytes(vg_lite_buffer_format_t format,
                             uint32_t *mul,
                             uint32_t *div,
                             uint32_t *bytes_align)
{
    *mul = *div = 1;
    *bytes_align = 4;
    switch (format) {
        case VG_LITE_L8:
        case VG_LITE_A8:
        case VG_LITE_RGBA8888_ETC2_EAC:
            break;

        case VG_LITE_A4:
            *div = 2;
            break;

        case VG_LITE_ABGR1555:
        case VG_LITE_ARGB1555:
        case VG_LITE_BGRA5551:
        case VG_LITE_RGBA5551:
        case VG_LITE_RGBA4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_ABGR4444:
        case VG_LITE_ARGB4444:
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_YUYV:
        case VG_LITE_YUY2:
        case VG_LITE_YUY2_TILED:
        /* AYUY2 buffer memory = YUY2 + alpha. */
        case VG_LITE_AYUY2:
        case VG_LITE_AYUY2_TILED:
        /* ABGR8565_PLANAR buffer memory = RGB565 + alpha. */
        case VG_LITE_ABGR8565_PLANAR:
        case VG_LITE_ARGB8565_PLANAR:
        case VG_LITE_RGBA5658_PLANAR:
        case VG_LITE_BGRA5658_PLANAR:
            *mul = 2;
            break;

        case VG_LITE_RGBA8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_XBGR8888:
        case VG_LITE_XRGB8888:
            *mul = 4;
            break;

        case VG_LITE_NV12:
        case VG_LITE_NV12_TILED:
            *mul = 3;
            break;

        case VG_LITE_ANV12:
        case VG_LITE_ANV12_TILED:
            *mul = 4;
            break;

        case VG_LITE_INDEX_1:
            *div = 8;
            *bytes_align = 8;
            break;

        case VG_LITE_INDEX_2:
            *div = 4;
            *bytes_align = 8;
            break;

        case VG_LITE_INDEX_4:
            *div = 2;
            *bytes_align = 8;
            break;

        case VG_LITE_INDEX_8:
            *bytes_align = 1;
            break;

        case VG_LITE_RGBA2222:
        case VG_LITE_BGRA2222:
        case VG_LITE_ABGR2222:
        case VG_LITE_ARGB2222:
            *mul = 1;
            break;

        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
        case VG_LITE_ABGR8565:
        case VG_LITE_BGRA5658:
        case VG_LITE_ARGB8565:
        case VG_LITE_RGBA5658:
            *mul = 3;
            break;

        /* OpenVG format*/
        case VG_sRGBX_8888:
        case VG_sRGBA_8888:
        case VG_sRGBA_8888_PRE:
        case VG_lRGBX_8888:
        case VG_lRGBA_8888:
        case VG_lRGBA_8888_PRE:
        case VG_sXRGB_8888:
        case VG_sARGB_8888:
        case VG_sARGB_8888_PRE:
        case VG_lXRGB_8888:
        case VG_lARGB_8888:
        case VG_lARGB_8888_PRE:
        case VG_sBGRX_8888:
        case VG_sBGRA_8888:
        case VG_sBGRA_8888_PRE:
        case VG_lBGRX_8888:
        case VG_lBGRA_8888:
        case VG_sXBGR_8888:
        case VG_sABGR_8888:
        case VG_lBGRA_8888_PRE:
        case VG_sABGR_8888_PRE:
        case VG_lXBGR_8888:
        case VG_lABGR_8888:
        case VG_lABGR_8888_PRE:
            *mul = 4;
            break;

        case VG_sRGBA_5551:
        case VG_sRGBA_4444:
        case VG_sARGB_1555:
        case VG_sARGB_4444:
        case VG_sBGRA_5551:
        case VG_sBGRA_4444:
        case VG_sABGR_1555:
        case VG_sABGR_4444:
        case VG_sRGB_565:
        case VG_sBGR_565:
            * mul = 2;
            break;

        case VG_sL_8:
        case VG_lL_8:
        case VG_A_8:
            break;

        case VG_BW_1:
        case VG_A_4:
        case VG_A_1:
            * div = 2;
            break;

        default:
            break;
    }
}

/* Convert VGLite target color format to HW value. */
static uint32_t convert_target_format(vg_lite_buffer_format_t format, vg_lite_capabilities_t caps)
{
    switch (format) {
        case VG_LITE_A8:
            return 0x0;

        case VG_LITE_L8:
            return 0x6;

        case VG_LITE_ABGR4444:
            return 0x14;

        case VG_LITE_ARGB4444:
            return 0x34;

        case VG_LITE_RGBA4444:
            return 0x24;

        case VG_LITE_BGRA4444:
            return 0x4;

        case VG_LITE_RGB565:
            return 0x21;

        case VG_LITE_BGR565:
            return 0x1;

        case VG_LITE_ABGR8888:
            return 0x13;

        case VG_LITE_ARGB8888:
            return 0x33;

        case VG_LITE_RGBA8888:
            return 0x23;

        case VG_LITE_BGRA8888:
            return 0x3;

        case VG_LITE_RGBX8888:
            return 0x22;

        case VG_LITE_BGRX8888:
            return 0x2;

        case VG_LITE_XBGR8888:
            return 0x12;

        case VG_LITE_XRGB8888:
            return 0x32;

        case VG_LITE_ABGR1555:
            return 0x15;

        case VG_LITE_RGBA5551:
            return 0x25;

        case VG_LITE_ARGB1555:
            return 0x35;

        case VG_LITE_BGRA5551:
            return 0x5;

        case VG_LITE_YUYV:
        case VG_LITE_YUY2:
        case VG_LITE_YUY2_TILED:
            return 0x8;

        case VG_LITE_NV12:
        case VG_LITE_NV12_TILED:
            return 0xB;

        case VG_LITE_ANV12:
        case VG_LITE_ANV12_TILED:
            return 0xE;

        case VG_LITE_BGRA2222:
            return 0x7;

        case VG_LITE_RGBA2222:
            return 0x27;

        case VG_LITE_ABGR2222:
            return 0x17;

        case VG_LITE_ARGB2222:
            return 0x37;

        case VG_LITE_ARGB8565:
            return 0x3A;

        case VG_LITE_RGBA5658:
            return 0x2A;

        case VG_LITE_ABGR8565:
            return 0x1A;

        case VG_LITE_BGRA5658:
            return 0x0A;

        case VG_LITE_ARGB8565_PLANAR:
            return 0x3C;

        case VG_LITE_RGBA5658_PLANAR:
            return 0x2C;

        case VG_LITE_ABGR8565_PLANAR:
            return 0x1C;

        case VG_LITE_BGRA5658_PLANAR:
            return 0x0C;

        case VG_LITE_RGB888:
            return 0x29;

        case VG_LITE_BGR888:
            return 0x09;

        case VG_LITE_AYUY2:
        case VG_LITE_AYUY2_TILED:
            return 0xF;

        /* OpenVG VGImageFormat */

        case VG_sRGBX_8888:
            return 0x12;
            break;

        case VG_sRGBA_8888:
        case VG_sRGBA_8888_PRE:
            return 0x13;
            break;

        case VG_sRGB_565:
            return 0x1;
            break;

        case VG_sRGBA_5551:
            return 0x15;
            break;

        case VG_sRGBA_4444:
            return 0x14;
            break;

        case VG_sL_8:
            return 0x6;
            break;

        case VG_lRGBX_8888:
            return 0x12;
            break;

        case VG_lRGBA_8888:
        case VG_lRGBA_8888_PRE:
            return 0x13;
            break;

        case VG_lL_8:
            return 0x6;
            break;

        case VG_A_8:
            return 0x0;
            break;

        case VG_sXRGB_8888:
            return 0x2;
            break;

        case VG_sARGB_8888:
            return 0x3;
            break;

        case VG_sARGB_8888_PRE:
            return 0x3;
            break;

        case VG_sARGB_1555:
            return 0x5;
            break;

        case VG_sARGB_4444:
            return 0x4;
            break;

        case VG_lXRGB_8888:
            return 0x2;
            break;

        case VG_lARGB_8888:
            return 0x3;
            break;

        case VG_lARGB_8888_PRE:
            return 0x3;
            break;

        case VG_sBGRX_8888:
            return 0x32;
            break;

        case VG_sBGRA_8888:
            return 0x33;
            break;

        case VG_sBGRA_8888_PRE:
            return 0x33;
            break;

        case VG_sBGR_565:
            return 0x21;
            break;

        case VG_sBGRA_5551:
            return 0x35;
            break;

        case VG_sBGRA_4444:
            return 0x34;
            break;

        case VG_lBGRX_8888:
            return 0x32;
            break;

        case VG_lBGRA_8888:
            return 0x33;
            break;

        case VG_lBGRA_8888_PRE:
            return 0x33;
            break;

        case VG_sXBGR_8888:
            return 0x22;
            break;

        case VG_sABGR_8888:
            return 0x23;
            break;

        case VG_sABGR_8888_PRE:
            return 0x23;
            break;

        case VG_sABGR_1555:
            return 0x5;
            break;

        case VG_sABGR_4444:
            return 0x24;
            break;

        case VG_lXBGR_8888:
            return 0x22;
            break;

        case VG_lABGR_8888:
            return 0x23;
            break;

        case VG_lABGR_8888_PRE:
            return 0x23;
            break;

        default:
            return 0xFF;
    }
}

#define FORMAT_ALIGNMENT(stride,align) \
    { \
        if ((stride) % (align) != 0) \
            return VG_LITE_INVALID_ARGUMENT; \
        return VG_LITE_SUCCESS; \
    }

/* determine source IM is aligned by specified bytes */
static vg_lite_error_t _check_source_aligned(vg_lite_buffer_format_t format,uint32_t stride)
{
    switch (format) {
        case VG_LITE_A4:
        case VG_LITE_INDEX_1:
        case VG_LITE_INDEX_2:
        case VG_LITE_INDEX_4:
            FORMAT_ALIGNMENT(stride,8);
            break;

        case VG_LITE_L8:
        case VG_LITE_A8:
        case VG_LITE_INDEX_8:
        case VG_LITE_RGBA2222:
        case VG_LITE_BGRA2222:
        case VG_LITE_ABGR2222:
        case VG_LITE_ARGB2222:
        case VG_LITE_RGBA8888_ETC2_EAC:
            FORMAT_ALIGNMENT(stride,16);
            break;

        case VG_LITE_RGBA4444:
        case VG_LITE_BGRA4444:
        case VG_LITE_ABGR4444:
        case VG_LITE_ARGB4444:
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_BGRA5551:
        case VG_LITE_RGBA5551:
        case VG_LITE_ABGR1555:
        case VG_LITE_ARGB1555:
        case VG_LITE_YUYV:
        case VG_LITE_YUY2:
        case VG_LITE_NV12:
        case VG_LITE_YV12:
        case VG_LITE_YV24:
        case VG_LITE_YV16:
        case VG_LITE_NV16:
        case VG_LITE_ABGR8565_PLANAR:
        case VG_LITE_BGRA5658_PLANAR:
        case VG_LITE_ARGB8565_PLANAR:
        case VG_LITE_RGBA5658_PLANAR:
            FORMAT_ALIGNMENT(stride,32);
            break;

        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
        case VG_LITE_ABGR8565:
        case VG_LITE_BGRA5658:
        case VG_LITE_ARGB8565:
        case VG_LITE_RGBA5658:
            FORMAT_ALIGNMENT(stride,48);
            break;

        case VG_LITE_RGBA8888:
        case VG_LITE_BGRA8888:
        case VG_LITE_ABGR8888:
        case VG_LITE_ARGB8888:
        case VG_LITE_RGBX8888:
        case VG_LITE_BGRX8888:
        case VG_LITE_XBGR8888:
        case VG_LITE_XRGB8888:
            FORMAT_ALIGNMENT(stride,64);
            break;

        default:
            return VG_LITE_SUCCESS;
    }
}

/* Convert VGLite source color format to HW values. */
uint32_t convert_source_format(vg_lite_buffer_format_t format)
{
    switch (format) {
        case VG_LITE_L8:
            return 0x0;

        case VG_LITE_A4:
            return 0x1;

        case VG_LITE_A8:
            return 0x2;

        case VG_LITE_RGBA4444:
            return 0x23;

        case VG_LITE_BGRA4444:
            return 0x3;

        case VG_LITE_ABGR4444:
            return 0x13;

        case VG_LITE_ARGB4444:
            return 0x33;

        case VG_LITE_RGB565:
            return 0x25;

        case VG_LITE_BGR565:
            return 0x5;

        case VG_LITE_RGBA8888:
            return 0x27;

        case VG_LITE_BGRA8888:
            return 0x7;

        case VG_LITE_ABGR8888:
            return 0x17;

        case VG_LITE_ARGB8888:
            return 0x37;

        case VG_LITE_RGBX8888:
            return 0x26;

        case VG_LITE_BGRX8888:
            return 0x6;

        case VG_LITE_XBGR8888:
            return 0x16;

        case VG_LITE_XRGB8888:
            return 0x36;

        case VG_LITE_BGRA5551:
            return 0x4;

        case VG_LITE_RGBA5551:
            return 0x24;

        case VG_LITE_ABGR1555:
            return 0x14;

        case VG_LITE_ARGB1555:
            return 0x34;

        case VG_LITE_YUYV:
            return 0x8;

        case VG_LITE_YUY2:
        case VG_LITE_YUY2_TILED:
            return 0x8;

        case VG_LITE_NV12:
        case VG_LITE_NV12_TILED:
            return 0xB;

        case VG_LITE_ANV12:
        case VG_LITE_ANV12_TILED:
            return 0xE;

        case VG_LITE_YV12:
            return 0x9;

        case VG_LITE_YV24:
            return 0xD;

        case VG_LITE_YV16:
            return 0xC;

        case VG_LITE_NV16:
            return 0xA;

        case VG_LITE_AYUY2:
        case VG_LITE_AYUY2_TILED:
            return 0xF;

        case VG_LITE_INDEX_1:
            return 0x200;

        case VG_LITE_INDEX_2:
            return 0x400;

        case VG_LITE_INDEX_4:
            return 0x600;

        case VG_LITE_INDEX_8:
            return 0x800;

        case VG_LITE_RGBA2222:
            return 0xA20;

        case VG_LITE_BGRA2222:
            return 0xA00;

        case VG_LITE_ABGR2222:
            return 0xA10;

        case VG_LITE_ARGB2222:
            return 0xA30;

        case VG_LITE_RGBA8888_ETC2_EAC:
            return 0xE00;

        case VG_LITE_ARGB8565:
            return 0x40000030;

        case VG_LITE_RGBA5658:
            return 0x40000020;

        case VG_LITE_ABGR8565:
            return 0x40000010;

        case VG_LITE_BGRA5658:
            return 0x40000000;

        case VG_LITE_RGB888:
            return 0x20000020;

        case VG_LITE_BGR888:
            return 0x20000000;

        case VG_LITE_ARGB8565_PLANAR:
            return 0x60000030;

        case VG_LITE_RGBA5658_PLANAR:
            return 0x60000020;

        case VG_LITE_ABGR8565_PLANAR:
            return 0x60000010;

        case VG_LITE_BGRA5658_PLANAR:
            return 0x60000000;

    /* OpenVG VGImageFormat */
        case VG_sRGBX_8888:
            return 0x16;
            break;

        case VG_sRGBA_8888:
        case VG_sRGBA_8888_PRE:
            return 0x17;
            break;

        case VG_sRGB_565:
            return 0x5;
            break;

        case VG_sRGBA_5551:
            return 0x14;
            break;

        case VG_sRGBA_4444:
            return 0x13;
            break;

        case VG_sL_8:
            return 0x0;
            break;

        case VG_lRGBX_8888:
            return 0x16;
            break;

        case VG_lRGBA_8888:
        case VG_lRGBA_8888_PRE:
            return 0x17;
            break;

        case VG_lL_8:
            return 0x0;
            break;

        case VG_A_8:
            return 0x2;
            break;

        case VG_BW_1:
            return 0x200;
            break;

        case VG_A_1:
            return 0x1;
            break;

        case VG_A_4:
            return 0x1;
            break;

        case VG_sXRGB_8888:
            return 0x6;
            break;

        case VG_sARGB_8888:
            return 0x7;
            break;

        case VG_sARGB_8888_PRE:
            return 0x7;
            break;

        case VG_sARGB_1555:
            return 0x4;
            break;

        case VG_sARGB_4444:
            return 0x3;
            break;

        case VG_lXRGB_8888:
            return 0x6;
            break;

        case VG_lARGB_8888:
            return 0x7;
            break;
        case VG_lARGB_8888_PRE:
            return 0x7;
            break;

        case VG_sBGRX_8888:
            return 0x36;
            break;

        case VG_sBGRA_8888:
            return 0x37;
            break;

        case VG_sBGRA_8888_PRE:
            return 0x37;
            break;

        case VG_sBGR_565:
            return 0x25;
            break;

        case VG_sBGRA_5551:
            return 0x34;
            break;

        case VG_sBGRA_4444:
            return 0x33;
            break;

        case VG_lBGRX_8888:
            return 0x36;
            break;

        case VG_lBGRA_8888:
            return 0x37;
            break;

        case VG_lBGRA_8888_PRE:
            return 0x37;
            break;

        case VG_sXBGR_8888:
            return 0x26;
            break;

        case VG_sABGR_8888:
            return 0x27;
            break;

        case VG_sABGR_8888_PRE:
            return 0x27;
            break;

        case VG_sABGR_1555:
            return 0x24;
            break;

        case VG_sABGR_4444:
            return 0x23;
            break;

        case VG_lXBGR_8888:
            return 0x26;
            break;

        case VG_lABGR_8888:
            return 0x27;
            break;

        case VG_lABGR_8888_PRE:
            return 0x27;
            break;

        default:
            return 0;
            break;
    }
}

/* Convert VGLite blend modes to HW values. */
uint32_t convert_blend(vg_lite_blend_t blend)
{
    switch (blend) {
        case VG_LITE_BLEND_SRC_OVER:
        case VG_LITE_BLEND_PREMULTIPLY_SRC_OVER:
            return 0x00000100;
            
        case VG_LITE_BLEND_DST_OVER:
            return 0x00000200;
            
        case VG_LITE_BLEND_SRC_IN:
            return 0x00000300;
            
        case VG_LITE_BLEND_DST_IN:
            return 0x00000400;
            
        case VG_LITE_BLEND_SCREEN:
            return 0x00000600;
            
        case VG_LITE_BLEND_MULTIPLY:
            return 0x00000500;
            
        case VG_LITE_BLEND_ADDITIVE:
            return 0x00000900;
            
        case VG_LITE_BLEND_SUBTRACT:
            return 0x00000A00;

        case VG_LITE_BLEND_DARKEN:
            return 0x00000700;

        case VG_LITE_BLEND_LIGHTEN:
            return 0x00000800;

        case VG_LITE_BLEND_SUBTRACT_LVGL:
            return 0x00000C00;

        case VG_LITE_BLEND_NORMAL_LVGL:
            return 0x00000100;

        case VG_LITE_BLEND_ADDITIVE_LVGL:
            return 0x00000900;

        case VG_LITE_BLEND_MULTIPLY_LVGL:
            return 0x00000500;

        default:
            return 0;
    }
}

/* Convert VGLite uv swizzle enums to HW values. */
uint32_t convert_uv_swizzle(vg_lite_swizzle_t swizzle)
{
    switch (swizzle) {
        case VG_LITE_SWIZZLE_UV:
            return 0x00000040;
            break;
            
        case VG_LITE_SWIZZLE_VU:
            return 0x00000050;
            
        default:
            return 0;
            break;
    }
}

/* Convert VGLite yuv standard enums to HW values. */
uint32_t convert_yuv2rgb(vg_lite_yuv2rgb_t yuv)
{
    switch (yuv) {
        case VG_LITE_YUV601:
            return 0;
            break;
            
        case VG_LITE_YUV709:
            return 0x00008000;
            
        default:
            return 0;
            break;
    }
}

static vg_lite_error_t submit(vg_lite_context_t * context);
static vg_lite_error_t stall(vg_lite_context_t * context, uint32_t time_ms, uint32_t mask);

/* Push a state array into current command buffer. */
vg_lite_error_t push_clut(vg_lite_context_t * context, uint32_t address, uint32_t count, uint32_t *data)
{
    uint32_t i;
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    if (CMDBUF_OFFSET(*context) + 8 + VG_LITE_ALIGN(count + 1, 2) * 4 >= CMDBUF_SIZE(*context)) {
        VG_LITE_RETURN_ERROR(submit(context));
        VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
    }

    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_STATES(count, address);
    
    for (i = 0; i < count; i++) {
        ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1 + i] = data[i];
    }
    if (i%2 == 0) {
        ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1 + i] = VG_LITE_NOP();
    }

#if DUMP_COMMAND
    {
        uint32_t loops;
        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");

        fprintf(fp, "Command buffer: 0x%08x, ",
                ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0]);

        for (loops = 0; loops < count / 2; loops++) {
            fprintf(fp, "0x%08x,\nCommand buffer: 0x%08x, ",
                    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2 - 1],
                    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2]);
        }

        fprintf(fp, "0x%08x,\n",
                ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2 - 1]);

        fclose(fp);
        fp = NULL;
    }
#endif

    CMDBUF_OFFSET(*context) += VG_LITE_ALIGN(count + 1, 2) * 4;

    return VG_LITE_SUCCESS;
}

/* Push a single state command into the current command buffer. */
vg_lite_error_t push_state(vg_lite_context_t * context, uint32_t address, uint32_t data)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    /* TODO wait for hw to complete development. */
    /* if (address == 0x0A1B || context->hw.hw_states[address & 0xff].state != data || !context->hw.hw_states[address & 0xff].init) { */
        if (CMDBUF_OFFSET(*context) + 16 >= CMDBUF_SIZE(*context)) {
            VG_LITE_RETURN_ERROR(submit(context));
            VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
        }

        /* TODO context->hw.hw_states[address & 0xff].state = data;
        context->hw.hw_states[address & 0xff].init = 1;*/

        ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_STATE(address);
        ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = data;

#if DUMP_COMMAND
        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");

        fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0],
                ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1]);

        fclose(fp);
        fp = NULL;
#endif

        CMDBUF_OFFSET(*context) += 8;
    /* } */

    return VG_LITE_SUCCESS;
}

/* Push a single state command with given address. */
vg_lite_error_t push_state_ptr(vg_lite_context_t * context, uint32_t address, void * data_ptr)
{
    vg_lite_error_t error;
    uint32_t data = *(uint32_t *) data_ptr;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    /* TODO wait for hw to complete development. */
    /* if (address == 0x0A1B || context->hw.hw_states[address & 0xff].state != data || !context->hw.hw_states[address & 0xff].init) { */
        if (CMDBUF_OFFSET(*context) + 16 >= CMDBUF_SIZE(*context)) {
            VG_LITE_RETURN_ERROR(submit(context));
            VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
        }

        /* TODO context->hw.hw_states[address & 0xff].state = data;
        context->hw.hw_states[address & 0xff].init = 1;*/

        ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_STATE(address);
        ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = data;

#if DUMP_COMMAND
        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");
        fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0],
                ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1]);

        fclose(fp);
        fp = NULL;
#endif

        CMDBUF_OFFSET(*context) += 8;
    /* } */
    return VG_LITE_SUCCESS;
}

/* Push a "call" command into the current command buffer. */
vg_lite_error_t push_call(vg_lite_context_t * context, uint32_t address, uint32_t bytes)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    if (CMDBUF_OFFSET(*context) + 16 >= CMDBUF_SIZE(*context)) {
        VG_LITE_RETURN_ERROR(submit(context));
        VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
    }

    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_CALL((bytes + 7) / 8);
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = address;

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");
    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0],
            ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1]);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 8;

    return VG_LITE_SUCCESS;
}

#if gcFEATURE_VG_PE_CLEAR
static vg_lite_error_t push_pe_clear(vg_lite_context_t * context, uint32_t size)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    if (CMDBUF_OFFSET(*context) + 16 >= CMDBUF_SIZE(*context)) {
        VG_LITE_RETURN_ERROR(submit(context));
        VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
    }

    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_DATA(1);
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[2] = size;
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[3] = 0;

    CMDBUF_OFFSET(*context) += 16;

    return VG_LITE_SUCCESS;
}
#endif

/* Push a rectangle command into the current command buffer. */
static vg_lite_error_t push_rectangle(vg_lite_context_t * context, int32_t x, int32_t y, int32_t width, int32_t height)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    if (CMDBUF_OFFSET(*context) + 16 >= CMDBUF_SIZE(*context)) {
        VG_LITE_RETURN_ERROR(submit(context));
        VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
    }

    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_DATA(1);
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[4] = (uint16_t)x;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[5] = (uint16_t)y;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[6] = (uint16_t)width;
    ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[7] = (uint16_t)height;

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[5] << 16 |
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[4],
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[7] << 16 |
            ((uint16_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[6]);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 16;

    return VG_LITE_SUCCESS;
}

/* Push a data array into the current command buffer. */
vg_lite_error_t push_data(vg_lite_context_t * context, uint32_t size, void * data)
{
    vg_lite_error_t error;
    uint32_t bytes = VG_LITE_ALIGN(size, 8);

    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    if (CMDBUF_OFFSET(*context) + 16 + bytes >= CMDBUF_SIZE(*context)) {
        VG_LITE_RETURN_ERROR(submit(context));
        VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
    }

    if ((bytes + 8) > CMDBUF_SIZE(*context)) {
        printf("Command buffer size needs increase for data sized %d bytes!\n", bytes);
        return VG_LITE_OUT_OF_RESOURCES;
    }

    ((uint64_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(bytes / 8)] = 0;
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_DATA(bytes / 8);
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    memcpy(CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context) + 8, data, size);

#if DUMP_COMMAND
    {
        int32_t loops;

        if (strncmp(filename, "Commandbuffer", 13)) {
            sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
        }

        fp = fopen(filename, "a");

        if (fp == NULL)
            printf("error!\n");

        fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);
        for (loops = 0; loops < bytes / 8; loops++) {
            fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
                   ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2],
                   ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[(loops + 1) * 2 + 1]);
        }

        fclose(fp);
        fp = NULL;
    }
#endif

    CMDBUF_OFFSET(*context) += 8 + bytes;

    return VG_LITE_SUCCESS;
}

/* Push a "stall" command into the current command buffer. */
vg_lite_error_t push_stall(vg_lite_context_t * context, uint32_t module)
{
    vg_lite_error_t error;
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    if (CMDBUF_OFFSET(*context) + 16 >= CMDBUF_SIZE(*context)) {
        VG_LITE_RETURN_ERROR(submit(context));
        VG_LITE_RETURN_ERROR(stall(context, 0, (uint32_t)~0));
    }

    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_SEMAPHORE(module);
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[2] = VG_LITE_STALL(module);
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[3] = 0;

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);
    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[2], 0);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 16;

    return VG_LITE_SUCCESS;
}

/* Submit the current command buffer to HW and reset the current command buffer offset. */
static vg_lite_error_t submit(vg_lite_context_t *context)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_submit_t submit;

    /* Check if there is a valid context and an allocated command buffer. */
    if (!has_valid_command_buffer(context))
        return VG_LITE_NO_CONTEXT;

    /* Check if there is anything to submit. */
    if (CMDBUF_OFFSET(*context) == 0)
        return VG_LITE_INVALID_ARGUMENT;

    /* Check if there is enough space in the command buffer for the END. */
    if (CMDBUF_OFFSET(*context) + 8 > CMDBUF_SIZE(*context)) {
        /* Reset command buffer offset. */
        CMDBUF_OFFSET(*context) = 0;
        return VG_LITE_OUT_OF_RESOURCES;
    }

    /* Append END command into the command buffer. */
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0] = VG_LITE_END(EVENT_END);
    ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[1] = 0;

#if DUMP_COMMAND
    if (strncmp(filename, "Commandbuffer", 13)) {
        sprintf(filename, "Commandbuffer_pid%d.txt", getpid());
    }

    fp = fopen(filename, "a");

    if (fp == NULL)
        printf("error!\n");

    fprintf(fp, "Command buffer: 0x%08x, 0x%08x,\n",
            ((uint32_t *) (CMDBUF_BUFFER(*context) + CMDBUF_OFFSET(*context)))[0], 0);

    fprintf(fp, "Command buffer addr is : %p,\n", CMDBUF_BUFFER(*context));
    fprintf(fp, "Command buffer offset is : %d,\n", CMDBUF_OFFSET(*context) + 8);

    fclose(fp);
    fp = NULL;
#endif

    CMDBUF_OFFSET(*context) += 8;

    /* Submit the command buffer. */
    submit.context = &context->context;
    submit.commands = CMDBUF_BUFFER(*context);
    submit.command_size = CMDBUF_OFFSET(*context);
    submit.command_id = CMDBUF_INDEX(*context);

    /* Wait if GPU has not completed previous CMD buffer */
    if (submit_flag)
    {
        VG_LITE_RETURN_ERROR(stall(&s_context, 0, (uint32_t)~0));
    }
    // flush
#define kd_mpi_sys_mmz_flush_cache(a, b, c) (void)0
    kd_mpi_sys_mmz_flush_cache(0, submit.commands, submit.command_size);
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_SUBMIT, &submit));

    submit_flag = 1;

    vglitemDUMP_BUFFER("command", (size_t)CMDBUF_BUFFER(*context),
        submit.context->command_buffer_logical[CMDBUF_INDEX(*context)], 0, submit.command_size);
    vglitemDUMP("@[commit]");

    /* Reset command buffer. */
    CMDBUF_OFFSET(*context) = 0;

    return error;
}

/* Wait for the HW to finish the current execution. */
static vg_lite_error_t stall(vg_lite_context_t * context, uint32_t time_ms, uint32_t mask)
{
#if !defined(_WINDLL)
    vg_lite_error_t error;
#endif
    vg_lite_kernel_wait_t wait;

    vglitemDUMP("@[stall]");
    /* Wait until GPU is ready. */
    wait.context = &context->context;
    wait.timeout_ms = time_ms > 0 ? time_ms : VG_LITE_INFINITE;
    wait.event_mask = mask;
#if defined(_WINDLL)
    vg_lite_kernel(VG_LITE_WAIT, &wait);
#else
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_WAIT, &wait));
#endif

    submit_flag = 0;
    return VG_LITE_SUCCESS;
}

/* Get the inversion of a matrix. */
uint32_t inverse(vg_lite_matrix_t * result, vg_lite_matrix_t * matrix)
{
    vg_lite_float_t det00, det01, det02;
    vg_lite_float_t d;
    int32_t isAffine;

    /* Test for identity matrix. */
    if (matrix == NULL) {
        result->m[0][0] = 1.0f;
        result->m[0][1] = 0.0f;
        result->m[0][2] = 0.0f;
        result->m[1][0] = 0.0f;
        result->m[1][1] = 1.0f;
        result->m[1][2] = 0.0f;
        result->m[2][0] = 0.0f;
        result->m[2][1] = 0.0f;
        result->m[2][2] = 1.0f;

        /* Success. */
        return 1;
    }

    det00 = (matrix->m[1][1] * matrix->m[2][2]) - (matrix->m[2][1] * matrix->m[1][2]);
    det01 = (matrix->m[2][0] * matrix->m[1][2]) - (matrix->m[1][0] * matrix->m[2][2]);
    det02 = (matrix->m[1][0] * matrix->m[2][1]) - (matrix->m[2][0] * matrix->m[1][1]);
    
    /* Compute determinant. */
    d = (matrix->m[0][0] * det00) + (matrix->m[0][1] * det01) + (matrix->m[0][2] * det02);

    /* Return 0 if there is no inverse matrix. */
    if (d == 0.0f)
        return 0;

    /* Compute reciprocal. */
    d = 1.0f / d;

    /* Determine if the matrix is affine. */
    isAffine = (matrix->m[2][0] == 0.0f) && (matrix->m[2][1] == 0.0f) && (matrix->m[2][2] == 1.0f);

    result->m[0][0] = d * det00;
    result->m[0][1] = d * ((matrix->m[2][1] * matrix->m[0][2]) - (matrix->m[0][1] * matrix->m[2][2]));
    result->m[0][2] = d * ((matrix->m[0][1] * matrix->m[1][2]) - (matrix->m[1][1] * matrix->m[0][2]));
    result->m[1][0] = d * det01;
    result->m[1][1] = d * ((matrix->m[0][0] * matrix->m[2][2]) - (matrix->m[2][0] * matrix->m[0][2]));
    result->m[1][2] = d * ((matrix->m[1][0] * matrix->m[0][2]) - (matrix->m[0][0] * matrix->m[1][2]));
    result->m[2][0] = isAffine ? 0.0f : d * det02;
    result->m[2][1] = isAffine ? 0.0f : d * ((matrix->m[2][0] * matrix->m[0][1]) - (matrix->m[0][0] * matrix->m[2][1]));
    result->m[2][2] = isAffine ? 1.0f : d * ((matrix->m[0][0] * matrix->m[1][1]) - (matrix->m[1][0] * matrix->m[0][1]));

    /* Success. */
    return 1;
}

/* Transform a 2D point by a given matrix. */
uint32_t transform(vg_lite_point_t * result, vg_lite_float_t x, vg_lite_float_t y, vg_lite_matrix_t * matrix)
{
    vg_lite_float_t pt_x;
    vg_lite_float_t pt_y;
    vg_lite_float_t pt_w;
    
    /* Test for identity matrix. */
    if (matrix == NULL) {
        result->x = (int)x;
        result->y = (int)y;
        
        /* Success. */
        return 1;
    }

#if (CHIPID == 0x355)
    if (s_context.from_blit_rect) {
        if (x != 0) {
            x = x - 1;
        }
    }
#endif

    if (((matrix->m[0][1] != 0.0f) || (matrix->m[1][0] != 0.0f) || (matrix->m[2][0] != 0.0f) || (matrix->m[2][1] != 0.0f) || (matrix->m[2][2] != 1.0f)) &&
        (s_context.filter == VG_LITE_FILTER_LINEAR || s_context.filter == VG_LITE_FILTER_BI_LINEAR)) {
        if (x != 0) {
            x = x + 0.5f;
        }

        if (y != 0 && s_context.filter == VG_LITE_FILTER_BI_LINEAR) {
            y = y + 0.5f;
        }
    }
    
    /* Transform x, y, and w. */
    pt_x = (x * matrix->m[0][0]) + (y * matrix->m[0][1]) + matrix->m[0][2];
    pt_y = (x * matrix->m[1][0]) + (y * matrix->m[1][1]) + matrix->m[1][2];
    pt_w = (x * matrix->m[2][0]) + (y * matrix->m[2][1]) + matrix->m[2][2];
    
    if (pt_w <= 0.0f)
        return 0;
    
    /* Compute projected x and y. */
    result->x = (int)((pt_x / pt_w) + 0.5f);
    result->y = (int)((pt_y / pt_w) + 0.5f);
    
    /* Success. */
    return 1;
}

/*!
 Flush specific VG module.
 */
static vg_lite_error_t flush_target()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_context_t *context = GET_CONTEXT();
    
    do {
        VG_LITE_BREAK_ERROR(push_state(context, 0x0A1B, 0x00000001));
        VG_LITE_BREAK_ERROR(push_stall(context, 7));
    } while (0);
    
    return error;
}

/* Set the current render target. */
vg_lite_error_t set_render_target(vg_lite_buffer_t *target)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    uint32_t yuv2rgb = 0;
    uint32_t uv_swiz = 0;
    int32_t tiled;
    int32_t stride;
    uint8_t flexa_mode = 0;
    uint32_t compress_mode = 0;
    uint32_t mirror_mode = 0;
    uint32_t premultiply_dst = 0;
    uint32_t rgb_alphadiv = 0;
    uint32_t read_dest = 0;
    uint32_t dst_format = 0;
    uint32_t gamma_value = 0;

    if (target == NULL)
        return VG_LITE_INVALID_ARGUMENT;

    /* Skip if render target and scissor are not changed. */
    if ((s_context.rtbuffer != NULL) &&
        !(memcmp(s_context.rtbuffer,target,sizeof(vg_lite_buffer_t))) &&
        (s_context.scissor_dirty == 0) && (s_context.mirror_dirty == 0) &&
        (s_context.gamma_dirty == 0) && (s_context.premultiply_dirty == 0))
    {
        return VG_LITE_SUCCESS;
    }

#if gcFEATURE_VG_ERROR_CHECK
#if !gcFEATURE_VG_YUV_OUTPUT
    if ((target != NULL) &&
        (target->format == VG_LITE_YUY2 ||
         target->format == VG_LITE_AYUY2 ||
         target->format == VG_LITE_YUY2_TILED ||
         target->format == VG_LITE_AYUY2_TILED)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_24BIT_PLANAR
    if (target->format >= VG_LITE_ABGR8565_PLANAR && target->format <= VG_LITE_RGBA5658_PLANAR) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_RECTANGLE_TILED_OUT
    if (target->tiled != VG_LITE_LINEAR) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#endif /* gcFEATURE_VG_ERROR_CHECK */

    /* Flush target if necessary when switching. */
    if (s_context.rtbuffer && s_context.rtbuffer->memory) {
        vg_lite_finish();
    }

#if gcFEATURE_VG_IM_FASTCLEAR
    update_fc_buffer(target);
#endif

    tiled = (target->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0;
    
    if (((target->format >= VG_LITE_YUY2) &&
         (target->format <= VG_LITE_AYUY2)) ||
        ((target->format >= VG_LITE_YUY2_TILED) &&
         (target->format <= VG_LITE_AYUY2_TILED)))
    {
        yuv2rgb = convert_yuv2rgb(target->yuv.yuv2rgb);
        uv_swiz = convert_uv_swizzle(target->yuv.swizzle);
    }

    s_context.target_width = target->width;
    s_context.target_height = target->height;
    /* Program render target. */
    if (s_context.rtbuffer != target || memcmp(s_context.rtbuffer,target,sizeof(vg_lite_buffer_t)) || (s_context.flexa_dirty != 0) ||
       (s_context.mirror_dirty != 0) || (s_context.gamma_dirty != 0) || (s_context.premultiply_dirty != 0)) {
        VG_LITE_RETURN_ERROR(check_compress(target->format, target->compress_mode, target->tiled, target->width, target->height));
        if (target->tiled == VG_LITE_TILED) {
            if ((target->stride % DEST_ALIGNMENT_LIMITATION) != 0)
                return VG_LITE_INVALID_ARGUMENT;
        }

        if (s_context.flexa_mode)
        {
            flexa_mode = 1 << 7;
        }
        if (s_context.mirror_orient)
        {
            mirror_mode = 1 << 16;
        }
        compress_mode = (uint32_t)target->compress_mode << 25;

#if gcFEATURE_VG_HW_PREMULTIPLY
        if (s_context.premultiply_dst) {
            premultiply_dst = 0x00000100;
        }
        rgb_alphadiv = 0x00000200;
#else
        premultiply_dst = 0x00000100;

#if (!gcFEATURE_VG_SRC_PREMULTIPLIED && CHIPID == 0x265)
        /* In the new version of 265, HW requirements PRE_MULTIPLED to be set to 0 to achieve HW internal premultiplication. */
        if (s_context.blend_mode != VG_LITE_BLEND_NONE && s_context.blend_mode != VG_LITE_BLEND_PREMULTIPLY_SRC_OVER) {
            premultiply_dst = 0x00000000;
        }
#endif
#endif

#if gcFEATURE_VG_USE_DST
        read_dest = 0x00100000;
#endif

#if gcFEATURE_VG_GAMMA
        /* Set gamma configuration of dst buffer */
        if ((target->format >= VG_sRGBX_8888 && target->format <= VG_sL_8) ||
            (target->format >= VG_sXRGB_8888 && target->format <= VG_sARGB_4444) ||
            (target->format >= VG_sBGRX_8888 && target->format <= VG_sBGRA_4444) ||
            (target->format >= VG_sXBGR_8888 && target->format <= VG_sABGR_4444))
        {
            s_context.gamma_dst = 1;
        }
        else
        {
            s_context.gamma_dst = 0;
        }

        if (s_context.gamma_src == 0 && s_context.gamma_dst == 1)
        {
            gamma_value = 0x00002000;
        }
        else if (s_context.gamma_src == 1 && s_context.gamma_dst == 0)
        {
            gamma_value = 0x00001000;
        }
        else
        {
            gamma_value = s_context.gamma_value;
        }
#endif

#if (CHIPID == 0x355)
        if (target->format == VG_LITE_L8 || target->format == VG_LITE_YUYV || target->format == VG_LITE_BGRA2222 || target->format == VG_LITE_RGBA2222 ||
            target->format == VG_LITE_ABGR2222 || target->format == VG_LITE_ARGB2222)
            return error;
#endif

        dst_format = convert_target_format(target->format, s_context.capabilities);
        if (dst_format == 0xFF) {
            printf("error: Target format is not supported.\n");
            return VG_LITE_INVALID_ARGUMENT;
        }

        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A10,
            dst_format | read_dest | uv_swiz | yuv2rgb | flexa_mode | compress_mode | mirror_mode | gamma_value | premultiply_dst | rgb_alphadiv));

        s_context.mirror_dirty = 0;
        s_context.gamma_dirty = 0;
        s_context.premultiply_dirty = 0;

        if (s_context.flexa_dirty  && !s_context.flexa_mode && s_context.tessbuf.tessbuf_size) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AC8, s_context.tessbuf.tessbuf_size -64));
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AC8, s_context.tessbuf.tessbuf_size));
            s_context.flexa_dirty = 0;
        }

        if (target->yuv.uv_planar)
        {   /* Program uv plane address if necessary. */
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5C, target->yuv.uv_planar));
        }
        if (target->yuv.alpha_planar) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A5D, target->yuv.alpha_planar));
        }
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A11, target->address));
        /* 24bit format stride configured to 4bpp. */
        if (target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) {
            stride = target->stride / 3 * 4;
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A12, stride | tiled));
        }
        else {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A12, target->stride | tiled));
        }
    }

    if (s_context.scissor_dirty != 0) {
        if (s_context.scissor_set) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A13, s_context.scissor[2] | (s_context.scissor[3] << 16)));
        }
        else {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A13, target->width | (target->height << 16)));
        }
        s_context.scissor_dirty = 0;
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A13, target->width | (target->height << 16)));
    }

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("    set_render_target %p (%d, %d)\n", target, target->width, target->height);
#endif

    if (s_context.rtbuffer)
    {
        memcpy(s_context.rtbuffer, target, sizeof(vg_lite_buffer_t));
    }

    return error;
}

/*************** VGLite API Functions ***********************************************/

vg_lite_error_t vg_lite_clear(vg_lite_buffer_t * target,
                              vg_lite_rectangle_t * rect,
                              vg_lite_color_t color)
{
    vg_lite_error_t error;
    int32_t x, y, width, height;
    uint32_t color32;
    uint32_t tiled;
    uint32_t stripe_mode = 0;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_clear %p %p 0x%08X\n", target, rect, color);
    if (rect) VGLITE_LOG("    Rect(%d, %d, %d, %d)\n", rect->x, rect->y, rect->width, rect->height);
#endif

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Get rectangle. */
    x = (rect != NULL) ? rect->x : 0;
    y = (rect != NULL) ? rect->y : 0;
    width  = (rect != NULL) ? rect->width : s_context.rtbuffer->width;
    height = (rect != NULL) ? rect->height : s_context.rtbuffer->height;
    
    /* Compute the valid rectangle. */
    if (x < 0)
    {
        width += x;
        x = 0;
    }
    if (y < 0)
    {
        height += y;
        y = 0;
    }
    
    if (s_context.scissor_set)
    {
        int32_t right, bottom;
        right = x + width;
        bottom = y + height;

        /* Bounds check. */
        if ((s_context.scissor[0] >= x + width) ||
            (s_context.scissor[2] <= x) ||
            (s_context.scissor[1] >= y + height) ||
            (s_context.scissor[3] <= y))
        {
            /* Do nothing. */
            return VG_LITE_SUCCESS;
        }
        /* Intersects the scissor and the rectangle. */
        x = (x > s_context.scissor[0] ? x : s_context.scissor[0]);
        y = (y > s_context.scissor[1] ? y : s_context.scissor[1]);
        right = (right < s_context.scissor[2]  ? right : s_context.scissor[2]);
        bottom = (bottom < s_context.scissor[3] ? bottom : s_context.scissor[3]);
        width = right - x;
        height = bottom - y;
    }
    if(width <= 0 || height <= 0)
        return VG_LITE_SUCCESS;
    /* Get converted color when target is in L8 format. */
    color32 = (target->format == VG_LITE_L8) ? rgb_to_l(color) : color;

    tiled = (target->tiled != VG_LITE_LINEAR) ? 0x40 : 0;

    if (target->compress_mode)
    {
        stripe_mode = 0x20000000;
    }

#if gcFEATURE_VG_IM_FASTCLEAR
    if ((rect == NULL) ||
        ((x == 0) && (y == 0)  &&
         (width == s_context.rtbuffer->width) &&
         (height  == s_context.rtbuffer->height))) {
            convert_color(s_context.rtbuffer->format, color32, &color32, NULL);
            clear_fc(&target->fc_buffer[0],(uint32_t)color32);
    }
    else
#endif
    {
        /* Setup the command buffer. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color32));

#if gcFEATURE_VG_PE_CLEAR
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A39, 0));
        if ((!rect || (x == 0 && y == 0 && width == target->width)) && !s_context.scissor_enable) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x10000004 | tiled | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable | stripe_mode));
            VG_LITE_RETURN_ERROR(push_pe_clear(&s_context, target->stride * height));
        }
        else
#endif
        {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x10000001 | tiled | s_context.scissor_enable | s_context.color_transform | s_context.matrix_enable | stripe_mode));
            VG_LITE_RETURN_ERROR(push_rectangle(&s_context, x, y, width, height));
        }

        /* flush VGPE after clear */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1B, 0x00000001));
    }

    /* Success. */
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_blit2(vg_lite_buffer_t* target,
                            vg_lite_buffer_t* source0,
                            vg_lite_buffer_t* source1,
                            vg_lite_matrix_t* matrix0,
                            vg_lite_matrix_t* matrix1,
                            vg_lite_blend_t blend,
                            vg_lite_filter_t filter)
{
#if gcFEATURE_VG_DOUBLE_IMAGE && gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error;
    vg_lite_point_t point_min, point_max, temp;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[2][3];
    vg_lite_float_t y_step[2][3];
    vg_lite_float_t c_step[2][3];
    uint32_t imageMode;
    uint32_t blend_mode;
    uint32_t filter_mode = 0;
    int32_t stride0;
    int32_t stride1;
    uint32_t rotation = 0;
    uint32_t conversion = 0;
    uint32_t tiled0, tiled1;
    int32_t left, right, bottom, top;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_blit2 %p %p %p %p %p %d %d\n", target, source0, source1, matrix0, matrix1, blend, filter);
#endif

#if gcFEATURE_VG_ERROR_CHECK
#if !gcFEATURE_VG_LVGL_SUPPORT
    if ((blend >= VG_LITE_BLEND_SUBTRACT_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_24BIT
    if ((target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) ||
        (source0->format >= VG_LITE_RGB888 && source0->format <= VG_LITE_RGBA5658) ||
        (source1->format >= VG_LITE_RGB888 && source1->format <= VG_LITE_RGBA5658)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_YUY2_INPUT
    if (source0->format == VG_LITE_YUYV || source0->format == VG_LITE_YUY2 || source1->format == VG_LITE_YUYV || source1->format == VG_LITE_YUY2) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_YUV_INPUT
    if ((source0->format >= VG_LITE_NV12 && source0->format <= VG_LITE_NV16) || (source1->format >= VG_LITE_NV12 && source1->format <= VG_LITE_NV16)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_AYUV_INPUT
    if (source0->format == VG_LITE_ANV12 || source0->format == VG_LITE_AYUY2 || source1->format == VG_LITE_ANV12 || source1->format == VG_LITE_AYUY2) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_YUV_TILED_INPUT
    if ((source0->format >= VG_LITE_YUY2_TILED && source0->format <= VG_LITE_AYUY2_TILED) || (source1->format >= VG_LITE_YUY2_TILED && source1->format <= VG_LITE_AYUY2_TILED)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_NEW_BLEND_MODE
    if (blend == VG_LITE_BLEND_DARKEN || blend == VG_LITE_BLEND_LIGHTEN) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#endif /* gcFEATURE_VG_ERROR_CHECK */

#if gcFEATURE_VG_LVGL_SUPPORT
    if (blend >= VG_LITE_BLEND_NORMAL_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) {
        vg_lite_dest_global_alpha(VG_LITE_GLOBAL, 0xFF);
    }
#endif

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Check if the specified matrix has rotation or perspective. */
    if (   (matrix0 != NULL)
        && (   (matrix0->m[0][1] != 0.0f)
            || (matrix0->m[1][0] != 0.0f)
            || (matrix0->m[2][0] != 0.0f)
            || (matrix0->m[2][1] != 0.0f)
            || (matrix0->m[2][2] != 1.0f)
            )
        ) {
        /* Mark that we have rotation. */
        rotation = 0x8000;
    }

    /* Check whether L8 is supported or not. */
    if ((target->format == VG_LITE_L8) && ((source0->format != VG_LITE_L8) && (source0->format != VG_LITE_A8))) {
        conversion = 0x80000000;
    }

    /* Calculate transformation for Image0 (Paint) & Image1 (Image). */
    /* Image1. */
    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Set initial point. */
    point_min = temp;
    point_max = temp;

    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)source0->height, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source0->width, (vg_lite_float_t)source0->height, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source0->width, 0.0f, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Clip to target. */
    if (s_context.scissor_set) {
        left = s_context.scissor[0];
        top = s_context.scissor[1];
        right = s_context.scissor[2];
        bottom = s_context.scissor[3];
    }
    else {
        left = top = 0;
        right = target->width;
        bottom = target->height;
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);

    if ((point_max.x - point_min.x) <= 0 || (point_max.y - point_min.y) <= 0)
        return VG_LITE_SUCCESS;
    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix0))
        return VG_LITE_INVALID_ARGUMENT;

    /* Compute interpolation steps for image1 (Image). */
    x_step[1][0] = inverse_matrix.m[0][0] / source0->width;
    x_step[1][1] = inverse_matrix.m[1][0] / source0->height;
    x_step[1][2] = inverse_matrix.m[2][0];
    y_step[1][0] = inverse_matrix.m[0][1] / source0->width;
    y_step[1][1] = inverse_matrix.m[1][1] / source0->height;
    y_step[1][2] = inverse_matrix.m[2][1];
    c_step[1][0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) + inverse_matrix.m[0][2]) / source0->width;
    c_step[1][1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]) / source0->height;
    c_step[1][2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];

    /* Image0 (Paint, as background ). */
    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Set initial point. */
    point_min = temp;
    point_max = temp;

    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)source1->height, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source1->width, (vg_lite_float_t)source1->height, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source1->width, 0.0f, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Clip to target. */
    if (s_context.scissor_set) {
        left = s_context.scissor[0];
        top = s_context.scissor[1];
        right = s_context.scissor[2];
        bottom = s_context.scissor[3];
    }
    else {
        left = top = 0;
        right = target->width;
        bottom = target->height;
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix1))
        return VG_LITE_INVALID_ARGUMENT;

    /* Compute interpolation steps for image1 (Image). */
    x_step[0][0] = inverse_matrix.m[0][0] / source1->width;
    x_step[0][1] = inverse_matrix.m[1][0] / source1->height;
    x_step[0][2] = inverse_matrix.m[2][0];
    y_step[0][0] = inverse_matrix.m[0][1] / source1->width;
    y_step[0][1] = inverse_matrix.m[1][1] / source1->height;
    y_step[0][2] = inverse_matrix.m[2][1];
    c_step[0][0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) + inverse_matrix.m[0][2]) / source1->width;
    c_step[0][1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]) / source1->height;
    c_step[0][2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];

    /* DOUBLE_IMAGE mode. */
    imageMode = 0x5000;
    blend_mode = convert_blend(blend);
    tiled0 = (source0->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0;
    tiled1 = (source1->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0;

    switch (filter) {
    case VG_LITE_FILTER_POINT:
        filter_mode = 0;
        break;

    case VG_LITE_FILTER_LINEAR:
        filter_mode = 0x10000;
        break;

    case VG_LITE_FILTER_BI_LINEAR:
        filter_mode = 0x20000;
        break;

    case VG_LITE_FILTER_GAUSSIAN:
        filter_mode = 0x30000;
        break;
    }

    /* Setup the command buffer. */
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, 0x10000001 | imageMode | blend_mode | rotation | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));
    /* Program image1. */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (void *) &c_step[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (void *) &c_step[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (void *) &c_step[1][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (void *) &x_step[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (void *) &x_step[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (void *) &x_step[1][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (void *) &y_step[1][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (void *) &y_step[1][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (void *) &y_step[1][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source0->format) | filter_mode | conversion));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source0->address));

    if (source0->yuv.uv_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source0->yuv.uv_planar));
    }
    if (source0->yuv.v_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source0->yuv.v_planar));
    }

    if (source0->yuv.alpha_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source0->yuv.alpha_planar));
    }
    /* 24bit format stride configured to 4bpp. */
    if (source0->format >= VG_LITE_RGB888 && source0->format <= VG_LITE_RGBA5658) {
        stride0 = source0->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, stride0 | tiled0));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source0->stride | tiled0));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, source0->width | (source0->height << 16)));

    /* Program image0 (Paint, as background). */
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A84, (void *) &c_step[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A85, (void *) &c_step[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A86, (void *) &c_step[0][2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A7C, (void *) &x_step[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A7D, (void *) &x_step[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A7E, (void *) &x_step[0][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A80, (void *) &y_step[0][0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A81, (void *) &y_step[0][1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A82, (void *) &y_step[0][2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A24, convert_source_format(source1->format) | filter_mode | conversion));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A26, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A28, source1->address));
    if (source1->yuv.uv_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A50, source1->yuv.uv_planar));
    }
    if (source1->yuv.v_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A52, source1->yuv.v_planar));
    }
    if (source1->yuv.alpha_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A52, source1->yuv.alpha_planar));
    }
    /* 24bit format stride configured to 4bpp. */
    if (source1->format >= VG_LITE_RGB888 && source1->format <= VG_LITE_RGBA5658) {
        stride1 = source1->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, stride1 | tiled1));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2A, source1->stride | tiled1));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2C, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2E, source1->width | (source1->height << 16)));

    VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min.x, point_min.y, point_max.x - point_min.x,
                                        point_max.y - point_min.y));
    flush_target();
    vglitemDUMP_BUFFER("image", (size_t)source0->address, source0->memory, 0, (source0->stride)*(source0->height));
    vglitemDUMP_BUFFER("image", (size_t)source1->address, source1->memory, 0, (source1->stride)*(source1->height));
#if DUMP_IMAGE
    dump_img(source0->memory, source0->width, source0->height, source0->format);
    dump_img(source1->memory, source1->width, source1->height, source1->format);
#endif

    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_blit(vg_lite_buffer_t* target,
                            vg_lite_buffer_t* source,
                            vg_lite_matrix_t* matrix,
                            vg_lite_blend_t blend,
                            vg_lite_color_t color,
                            vg_lite_filter_t filter)
{
    if (source->memory)
        kd_mpi_sys_mmz_flush_cache(0, source->memory, source->stride * source->height);
#if gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error;
    vg_lite_point_t point_min, point_max, temp;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    uint32_t imageMode = 0;
    uint32_t in_premult = 0;
    uint32_t blend_mode = blend;
    uint32_t filter_mode = 0;
    uint32_t transparency_mode = 0;
    uint32_t conversion = 0;
    uint32_t tiled_source;
    uint32_t tiled;
    uint32_t yuv2rgb = 0;
    uint32_t uv_swiz = 0;
    uint32_t ahb_read_split = 0;
    uint32_t compress_mode = 0;
    uint32_t src_premultiply_enable = 0;
    uint32_t index_endian = 0;
    uint32_t eco_fifo = 0;
    uint32_t stripe_mode = 0;
    int32_t  left, top, right, bottom;
    int32_t  stride;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_blit %p %p %p %d 0x%08X %d\n", target, source, matrix, blend, color, filter);
#endif

#if gcFEATURE_VG_ERROR_CHECK
#if !gcFEATURE_VG_LVGL_SUPPORT
    if ((blend >= VG_LITE_BLEND_SUBTRACT_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) || (source->image_mode == VG_LITE_RECOLOR_MODE)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_INDEX_ENDIAN
    if ((source->format >= VG_LITE_INDEX_1) && (source->format <= VG_LITE_INDEX_4) && source->index_endian) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_RECTANGLE_TILED_OUT
    if (target->tiled != VG_LITE_LINEAR) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_RGBA8_ETC2_EAC
    if (source->format == VG_LITE_RGBA8888_ETC2_EAC) {
        return VG_LITE_NOT_SUPPORT;
    }
#else
    if ((source->format == VG_LITE_RGBA8888_ETC2_EAC) && (source->width % 16 || source->height % 4)) {
        return VG_LITE_INVALID_ARGUMENT;
    }
#endif
#if !gcFEATURE_VG_YUY2_INPUT
    if (source->format == VG_LITE_YUYV || source->format == VG_LITE_YUY2) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_YUV_INPUT
    if (source->format >= VG_LITE_NV12 && source->format <= VG_LITE_NV16) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_AYUV_INPUT
    if (source->format == VG_LITE_ANV12 || source->format == VG_LITE_AYUY2) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_YUV_TILED_INPUT
    if (source->format >= VG_LITE_YUY2_TILED && source->format <= VG_LITE_AYUY2_TILED) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_24BIT
    if ((target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) ||
        (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_24BIT_PLANAR
    if (source->format >= VG_LITE_ABGR8565_PLANAR && source->format <= VG_LITE_RGBA5658_PLANAR) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_IM_DEC_INPUT
    if (source->compress_mode != VG_LITE_DEC_DISABLE) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_STENCIL
    if (source->image_mode == VG_LITE_STENCIL_MODE) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_NEW_BLEND_MODE
    if (blend == VG_LITE_BLEND_DARKEN || blend == VG_LITE_BLEND_LIGHTEN) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
    if (blend && (target->format == VG_LITE_YUYV || target->format == VG_LITE_YUY2 || target->format == VG_LITE_YUY2_TILED
        || target->format == VG_LITE_AYUY2 || target->format == VG_LITE_AYUY2_TILED)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif /* gcFEATURE_VG_ERROR_CHECK */

#if gcFEATURE_VG_LVGL_SUPPORT
    if (blend >= VG_LITE_BLEND_NORMAL_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) {
        vg_lite_dest_global_alpha(VG_LITE_GLOBAL, 0xFF);
    }
#endif
#if gcFEATURE_VG_INDEX_ENDIAN
    if ((source->format >= VG_LITE_INDEX_1) && (source->format <= VG_LITE_INDEX_4) && source->index_endian) {
        index_endian = 1 << 14;
    }
#endif
#if gcFEATURE_VG_STRIPE_MODE
    /* Enable fifo feature to share buffer between vg and ts to improve the rotation performance */
    eco_fifo = 1 << 7;
#endif

    VG_LITE_RETURN_ERROR(check_compress(source->format, source->compress_mode, source->tiled, source->width, source->height));

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);
    s_context.filter = filter;

    /* Check if the specified matrix has rotation or perspective. */
    if (   (matrix != NULL)
        && (   (matrix->m[0][1] != 0.0f)
            || (matrix->m[1][0] != 0.0f)
            || (matrix->m[2][0] != 0.0f)
            || (matrix->m[2][1] != 0.0f)
            || (matrix->m[2][2] != 1.0f)
            )
        && (   blend == VG_LITE_BLEND_NONE
            || blend == VG_LITE_BLEND_SRC_IN
            || blend == VG_LITE_BLEND_DST_IN
            )
        ) {
#if gcFEATURE_VG_BORDER_CULLING
            /* Mark that we have rotation. */
            transparency_mode = 0x8000;
#else
            blend_mode = VG_LITE_BLEND_SRC_OVER;
#endif
#if gcFEATURE_VG_STRIPE_MODE
            stripe_mode = 1 << 29;
#endif
    }

    /* Check whether L8 is supported or not. */
    if ((target->format == VG_LITE_L8) && ((source->format != VG_LITE_L8) && (source->format != VG_LITE_A8))) {
        conversion = 0x80000000;
    }

#if gcFEATURE_VG_16PIXELS_ALIGNED
    /* Check if source specify bytes are aligned */
    error = _check_source_aligned(source->format, source->stride);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }
#endif

#if gcFEATURE_VG_GAMMA
    /* Set gamma configuration of source buffer */
    if ((source->format >= VG_sRGBX_8888 && source->format <= VG_sL_8) ||
        (source->format >= VG_sXRGB_8888 && source->format <= VG_sARGB_4444) ||
        (source->format >= VG_sBGRX_8888 && source->format <= VG_sBGRA_4444) ||
        (source->format >= VG_sXBGR_8888 && source->format <= VG_sABGR_4444))
    {
        s_context.gamma_src = 1;
    }
    else
    {
        s_context.gamma_src = 0;
    }
#endif

    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Set initial point. */
    point_min = temp;
    point_max = temp;
    
    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)source->height, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;
    
    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source->width, (vg_lite_float_t)source->height, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;
    
    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)source->width, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Clip to target. */
    if (s_context.scissor_set) {
        left = s_context.scissor[0];
        top  = s_context.scissor[1];
        right= s_context.scissor[2];
        bottom = s_context.scissor[3];
    }
    else 
    {
        left = top = 0;
        right = target->width;
        bottom = target->height;
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);

    /* No need to draw. */
    if ((point_max.x - point_min.x) <= 0 || (point_max.y - point_min.y) <= 0)
        return VG_LITE_SUCCESS;

    /*blend input into context*/
    s_context.blend_mode = blend;
    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    if (filter == VG_LITE_FILTER_LINEAR)
    {
        /* Compute interpolation steps. */
        x_step[0] = (inverse_matrix.m[0][0] - 0.5f * inverse_matrix.m[2][0]) / source->width;
        x_step[1] = inverse_matrix.m[1][0] / source->height;
        x_step[2] = inverse_matrix.m[2][0];
        y_step[0] = (inverse_matrix.m[0][1] - 0.5f * inverse_matrix.m[2][1]) / source->width;
        y_step[1] = inverse_matrix.m[1][1] / source->height;
        y_step[2] = inverse_matrix.m[2][1];
        c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) - 0.25f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[0][2] - 0.5f * inverse_matrix.m[2][2]) / source->width;
        c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]) / source->height;
        c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];
    }
    else if (filter == VG_LITE_FILTER_BI_LINEAR)
    {
        /* Shift the linear sampling points to center of pixels to avoid pixel offset issue */
        x_step[0] = (inverse_matrix.m[0][0] - 0.5f * inverse_matrix.m[2][0]) / source->width;
        x_step[1] = (inverse_matrix.m[1][0] - 0.5f * inverse_matrix.m[2][0]) / source->height;
        x_step[2] = inverse_matrix.m[2][0];
        y_step[0] = (inverse_matrix.m[0][1] - 0.5f * inverse_matrix.m[2][1]) / source->width;
        y_step[1] = (inverse_matrix.m[1][1] - 0.5f * inverse_matrix.m[2][1]) / source->height;
        y_step[2] = inverse_matrix.m[2][1];
        c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) - 0.25f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[0][2] - 0.5f * inverse_matrix.m[2][2]) / source->width;
        c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) - 0.25f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[1][2] - 0.5f * inverse_matrix.m[2][2]) / source->height;
        c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];
    }
    else
    {
        /* Compute interpolation steps. */
        x_step[0] = inverse_matrix.m[0][0] / source->width;
        x_step[1] = inverse_matrix.m[1][0] / source->height;
        x_step[2] = inverse_matrix.m[2][0];
        y_step[0] = inverse_matrix.m[0][1] / source->width;
        y_step[1] = inverse_matrix.m[1][1] / source->height;
        y_step[2] = inverse_matrix.m[2][1];
        c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) + inverse_matrix.m[0][2]) / source->width;
        c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]) / source->height;
        c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];
    }

    /* Determine image mode (NORMAL, NONE , MULTIPLY or STENCIL) depending on the color. */
    switch (source->image_mode) {
        case VG_LITE_NONE_IMAGE_MODE:
            imageMode = 0x00000001;
            break;
        case VG_LITE_MULTIPLY_IMAGE_MODE:
            imageMode = 0x00002001;
            break;
        case VG_LITE_NORMAL_IMAGE_MODE:
        case VG_LITE_ZERO:
            imageMode = 0x00001001;
            break;
        case VG_LITE_STENCIL_MODE:
            imageMode = 0x00003001;
            break;
        case VG_LITE_RECOLOR_MODE:
            imageMode = 0x00006001;
            break;
    }

    switch (filter) {
    case VG_LITE_FILTER_POINT:
        filter_mode = 0;
        break;

    case VG_LITE_FILTER_LINEAR:
        filter_mode = 0x10000;
        break;

    case VG_LITE_FILTER_BI_LINEAR:
        filter_mode = 0x20000;
        break;

    case VG_LITE_FILTER_GAUSSIAN:
        filter_mode = 0x30000;
        break;
    }

    in_premult = 0x10000000;

#if (CHIPID==0x555 || CHIPID==0x265)
    switch (source->format) {
        case VG_LITE_A4:
        case VG_LITE_A8:
            in_premult = 0x00000000;
            break;
        default:
            break;
    };
    switch (target->format) {
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
            in_premult = 0x00000000;
            break;
        default:
            break;
    };
#endif
#if !gcFEATURE_VG_SRC_PREMULTIPLIED
#if (CHIPID == 0x265)
    if (blend != VG_LITE_BLEND_NONE) {
        in_premult = 0x00000000;
    }
#else
    if (blend_mode == VG_LITE_BLEND_PREMULTIPLY_SRC_OVER) {
        in_premult = 0x00000000;
    }
#endif
#endif

    blend_mode = convert_blend(blend_mode);
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;
    tiled = (target->tiled != VG_LITE_LINEAR) ? 0x40 : 0;
    if (blend_mode) {
        /* The hw bit for improve read image buffer performance when enable alpha blending. */
        ahb_read_split = 1 << 7;
    }

    compress_mode = (uint32_t)source->compress_mode << 25;
    if (source->compress_mode || target->compress_mode) {
        stripe_mode = 0x20000000;
    }

    /* Setup the command buffer. */
#if gcFEATURE_VG_GLOBAL_ALPHA
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD1, s_context.dst_alpha_mode | s_context.dst_alpha_value | s_context.src_alpha_mode | s_context.src_alpha_value));
#endif
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | imageMode | blend_mode | transparency_mode | tiled | s_context.enable_mask | s_context.color_transform | s_context.matrix_enable | eco_fifo | s_context.scissor_enable | stripe_mode));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (void *) &c_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (void *) &c_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (void *) &c_step[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (void *) &x_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (void *) &x_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (void *) &x_step[2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (void *) &y_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (void *) &y_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (void *) &y_step[2]));

    if (((source->format >= VG_LITE_YUY2) &&
         (source->format <= VG_LITE_AYUY2)) ||
        ((source->format >= VG_LITE_YUY2_TILED) &&
         (source->format <= VG_LITE_AYUY2_TILED))) {
            yuv2rgb = convert_yuv2rgb(source->yuv.yuv2rgb);
            uv_swiz = convert_uv_swizzle(source->yuv.swizzle);
    }

#if gcFEATURE_VG_HW_PREMULTIPLY
    if (s_context.premultiply_src) {
        src_premultiply_enable = 0x01000100;
    } else {
        src_premultiply_enable = 0x01000000;
    }
#else
    src_premultiply_enable = 0x01000000;
#endif

#if gcFEATURE_VG_IM_FASTCLEAR
    if (source->fc_enable) {
        uint32_t im_fc_enable = (source->fc_enable == 0) ? 0 : 0x800000;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) | filter_mode | uv_swiz | yuv2rgb | conversion | im_fc_enable | ahb_read_split | compress_mode | src_premultiply_enable | index_endian));
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0ACF, source->fc_buffer[0].address));   /* FC buffer address. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD0, source->fc_buffer[0].color));     /* FC clear value. */
    }
    else
#endif

#if (CHIPID == 0x355)
        if (source->format == VG_LITE_L8 || source->format == VG_LITE_YUYV || source->format == VG_LITE_BGRA2222 || source->format == VG_LITE_RGBA2222 ||
            source->format == VG_LITE_ABGR2222 || source->format == VG_LITE_ARGB2222)
            return error;
#endif

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) | filter_mode | uv_swiz | yuv2rgb | conversion | ahb_read_split | compress_mode | src_premultiply_enable | index_endian));

    if (source->yuv.uv_planar) {
        /* Program u plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source->yuv.uv_planar));
    }
    if (source->yuv.v_planar) {
        /* Program v plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.v_planar));
    }
    if (source->yuv.alpha_planar != 0) {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.alpha_planar));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));
    /* 24bit format stride configured to 4bpp. */
    if (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658) {
        stride = source->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, stride | tiled_source));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source->stride | tiled_source));
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, source->width | (source->height << 16)));
    VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min.x, point_min.y, point_max.x - point_min.x,
                                        point_max.y - point_min.y));

#if gcFEATURE_VG_STRIPE_MODE
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
#endif

    if (!s_context.flexa_mode)
        error = flush_target();

    vglitemDUMP_BUFFER("image", (size_t)source->address, source->memory, 0, (source->stride)*(source->height));

#if DUMP_IMAGE
    dump_img(source->memory, source->width, source->height, source->format);
#endif
    
    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

vg_lite_error_t vg_lite_blit_rect(vg_lite_buffer_t* target,
                                vg_lite_buffer_t* source,
                                vg_lite_rectangle_t* rect,
                                vg_lite_matrix_t* matrix,
                                vg_lite_blend_t blend,
                                vg_lite_color_t color,
                                vg_lite_filter_t filter)
{
#if gcFEATURE_VG_IM_INPUT
    vg_lite_error_t error;
    vg_lite_point_t point_min, point_max, temp;
    vg_lite_matrix_t inverse_matrix;
    vg_lite_float_t x_step[3];
    vg_lite_float_t y_step[3];
    vg_lite_float_t c_step[3];
    uint32_t imageMode = 0;
    uint32_t in_premult = 0;
    int32_t stride;
    uint32_t transparency_mode = 0;
    uint32_t blend_mode;
    uint32_t filter_mode = 0;
    vg_lite_blend_t forced_blending = blend;
    uint32_t conversion = 0;
    uint32_t tiled_source;
    int32_t left, top, right, bottom;
    uint32_t rect_x = 0, rect_y = 0, rect_w = 0, rect_h = 0;
    uint32_t tiled;
    uint32_t yuv2rgb = 0;
    uint32_t uv_swiz = 0;
    uint32_t ahb_read_split = 0;
    uint32_t compress_mode;
    uint32_t src_premultiply_enable = 0;
    uint32_t index_endian = 0;
    uint32_t eco_fifo = 0;
    uint32_t stripe_mode = 0;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_blit_rect %p %p %p %p %d 0x%08X %d\n", target, source, rect, matrix, blend, color, filter);
#endif

#if gcFEATURE_VG_ERROR_CHECK
#if !gcFEATURE_VG_LVGL_SUPPORT
    if ((blend >= VG_LITE_BLEND_SUBTRACT_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) || (source->image_mode == VG_LITE_RECOLOR_MODE)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_INDEX_ENDIAN
    if ((source->format >= VG_LITE_INDEX_1) && (source->format <= VG_LITE_INDEX_4) && source->index_endian) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_RECTANGLE_TILED_OUT
    if (target->tiled != VG_LITE_LINEAR) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_RGBA8_ETC2_EAC
    if (source->format == VG_LITE_RGBA8888_ETC2_EAC) {
        return VG_LITE_NOT_SUPPORT;
    }
#else
    if ((source->format == VG_LITE_RGBA8888_ETC2_EAC) && (source->width % 16 || source->height % 4)) {
        return VG_LITE_INVALID_ARGUMENT;
    }
#endif
#if !gcFEATURE_VG_YUY2_INPUT
    if (source->format == VG_LITE_YUYV || source->format == VG_LITE_YUY2) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_YUV_INPUT
    if (source->format >= VG_LITE_NV12 && source->format <= VG_LITE_NV16) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_AYUV_INPUT
    if (source->format == VG_LITE_ANV12 || source->format == VG_LITE_AYUY2) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_YUV_TILED_INPUT
    if (source->format >= VG_LITE_YUY2_TILED && source->format <= VG_LITE_AYUY2_TILED) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_24BIT
    if ((target->format >= VG_LITE_RGB888 && target->format <= VG_LITE_RGBA5658) ||
        (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_24BIT_PLANAR
    if (source->format >= VG_LITE_ABGR8565_PLANAR && source->format <= VG_LITE_RGBA5658_PLANAR) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_IM_DEC_INPUT
    if (source->compress_mode != VG_LITE_DEC_DISABLE) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_STENCIL
    if (source->image_mode == VG_LITE_STENCIL_MODE) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
#if !gcFEATURE_VG_NEW_BLEND_MODE
    if (blend == VG_LITE_BLEND_DARKEN || blend == VG_LITE_BLEND_LIGHTEN) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif
    if (blend && (target->format == VG_LITE_YUYV || target->format == VG_LITE_YUY2 || target->format == VG_LITE_YUY2_TILED
        || target->format == VG_LITE_AYUY2 || target->format == VG_LITE_AYUY2_TILED)) {
        return VG_LITE_NOT_SUPPORT;
    }
#endif /* gcFEATURE_VG_ERROR_CHECK */

#if gcFEATURE_VG_LVGL_SUPPORT
    if (blend >= VG_LITE_BLEND_NORMAL_LVGL && blend <= VG_LITE_BLEND_MULTIPLY_LVGL) {
        vg_lite_dest_global_alpha(VG_LITE_GLOBAL, 0xFF);
    }
#endif
#if gcFEATURE_VG_INDEX_ENDIAN
    if ((source->format >= VG_LITE_INDEX_1) && (source->format <= VG_LITE_INDEX_4) && source->index_endian) {
        index_endian = 1 << 14;
    }
#endif
#if gcFEATURE_VG_STRIPE_MODE
    /* Enable fifo feature to share buffer between vg and ts to improve the rotation performance */
    eco_fifo = 1 << 7;
#endif

    VG_LITE_RETURN_ERROR(check_compress(source->format, source->compress_mode, source->tiled, source->width, source->height));

    transparency_mode = (source->transparency_mode == VG_LITE_IMAGE_TRANSPARENT ? 0x8000:0);
    s_context.filter = filter;

    /* Check if the specified matrix has rotation or perspective. */
    if (   (matrix != NULL)
        && (   (matrix->m[0][1] != 0.0f)
            || (matrix->m[1][0] != 0.0f)
            || (matrix->m[2][0] != 0.0f)
            || (matrix->m[2][1] != 0.0f)
            || (matrix->m[2][2] != 1.0f)
            )
        && (   blend == VG_LITE_BLEND_NONE
            || blend == VG_LITE_BLEND_SRC_IN
            || blend == VG_LITE_BLEND_DST_IN
            )
        ) {
#if gcFEATURE_VG_BORDER_CULLING
            /* Mark that we have rotation. */
            transparency_mode = 0x8000;
#else
            blend_mode = VG_LITE_BLEND_SRC_OVER;
#endif
#if gcFEATURE_VG_STRIPE_MODE
            stripe_mode = 1 << 29;
#endif
    }

    /* Check whether L8 is supported or not. */
    if ((target->format == VG_LITE_L8) && ((source->format != VG_LITE_L8) && (source->format != VG_LITE_A8))) {
        conversion = 0x80000000;
    }

#if gcFEATURE_VG_16PIXELS_ALIGNED
    /* Check if source specify bytes are aligned */
    error = _check_source_aligned(source->format, source->stride);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }
#endif

    /* Set source region. */
    if (rect != NULL) {
        if (rect->x < 0)
            rect_x = 0;
        else
            rect_x = rect->x;

        if (rect->y < 0)
            rect_y = 0;
        else
            rect_y = rect->y;

#if (CHIPID == 0x355)
        rect_w = rect->width + 1;
        s_context.from_blit_rect = 1;
#else
        rect_w = rect->width;
#endif
        rect_h = rect->height;
        
        if ((rect_x > (uint32_t)source->width) || (rect_y > (uint32_t)source->height) ||
            (rect_w == 0) || (rect_h == 0))
        {
            /*No intersection*/
            return VG_LITE_INVALID_ARGUMENT;
        }

        if (rect_x + rect_w > (uint32_t)source->width)
        {
            rect_w = source->width - rect_x;
        }

        if (rect_y + rect_h > (uint32_t)source->height)
        {
            rect_h = source->height - rect_y;
        }
    }
    else {
        rect_x = rect_y = 0;
        rect_w = source->width;
        rect_h = source->height;
    }
    
    /* Transform image (0,0) to screen. */
    if (!transform(&temp, 0.0f, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    /* Set initial point. */
    point_min = temp;
    point_max = temp;
    
    /* Transform image (0,height) to screen. */
    if (!transform(&temp, 0.0f, (vg_lite_float_t)rect_h, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;
    
    /* Transform image (width,height) to screen. */
    if (!transform(&temp, (vg_lite_float_t)rect_w, (vg_lite_float_t)rect_h, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;
    
    /* Transform image (width,0) to screen. */
    if (!transform(&temp, (vg_lite_float_t)rect_w, 0.0f, matrix))
        return VG_LITE_INVALID_ARGUMENT;
    
    /* Determine min/max. */
    if (temp.x < point_min.x) point_min.x = temp.x;
    if (temp.y < point_min.y) point_min.y = temp.y;
    if (temp.x > point_max.x) point_max.x = temp.x;
    if (temp.y > point_max.y) point_max.y = temp.y;

    /* Clip to target. */
    if (s_context.scissor_set) {
        left = s_context.scissor[0];
        top  = s_context.scissor[1];
        right= s_context.scissor[2];
        bottom = s_context.scissor[3];
    }
    else 
    {
        left = top = 0;
        right = target->width;
        bottom = target->height;
    }

    point_min.x = MAX(point_min.x, left);
    point_min.y = MAX(point_min.y, top);
    point_max.x = MIN(point_max.x, right);
    point_max.y = MIN(point_max.y, bottom);

    if ((point_max.x - point_min.x) <= 0 || (point_max.y - point_min.y) <= 0)
        return VG_LITE_SUCCESS;

#if (CHIPID == 0x355)
    s_context.from_blit_rect = 0;
#endif

    /* Set gamma of source buffer */
    if ((source->format >= VG_sRGBX_8888 && source->format <= VG_sL_8) ||
        (source->format >= VG_sXRGB_8888 && source->format <= VG_sARGB_4444) ||
        (source->format >= VG_sBGRX_8888 && source->format <= VG_sBGRA_4444) ||
        (source->format >= VG_sXBGR_8888 && source->format <= VG_sABGR_4444))
    {
        s_context.gamma_src = 1;
    }
    else
    {
        s_context.gamma_src = 0;
    }

    error = set_render_target(target);
    if (error != VG_LITE_SUCCESS) {
        return error;
    }

    /* Compute inverse matrix. */
    if (!inverse(&inverse_matrix, matrix))
        return VG_LITE_INVALID_ARGUMENT;

    if (filter == VG_LITE_FILTER_LINEAR)
    {
        /* Compute interpolation steps. */
        x_step[0] = (inverse_matrix.m[0][0] - 0.5f * inverse_matrix.m[2][0]) / rect_w;
        x_step[1] = inverse_matrix.m[1][0] / rect_h;
        x_step[2] = inverse_matrix.m[2][0];
        y_step[0] = (inverse_matrix.m[0][1] - 0.5f * inverse_matrix.m[2][1]) / rect_w;
        y_step[1] = inverse_matrix.m[1][1] / rect_h;
        y_step[2] = inverse_matrix.m[2][1];
        c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) - 0.25f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[0][2] - 0.5f * inverse_matrix.m[2][2]) / rect_w;
        c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]) / rect_h;
        c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];
    }
    else if (filter == VG_LITE_FILTER_BI_LINEAR)
    {
        /* Shift the linear sampling points to center of pixels to avoid pixel offset issue */
        x_step[0] = (inverse_matrix.m[0][0] - 0.5f * inverse_matrix.m[2][0]) / rect_w;
        x_step[1] = (inverse_matrix.m[1][0] - 0.5f * inverse_matrix.m[2][0]) / rect_h;
        x_step[2] = inverse_matrix.m[2][0];
        y_step[0] = (inverse_matrix.m[0][1] - 0.5f * inverse_matrix.m[2][1]) / rect_w;
        y_step[1] = (inverse_matrix.m[1][1] - 0.5f * inverse_matrix.m[2][1]) / rect_h;
        y_step[2] = inverse_matrix.m[2][1];
        c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) - 0.25f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[0][2] - 0.5f * inverse_matrix.m[2][2]) / rect_w;
        c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) - 0.25f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[1][2] - 0.5f * inverse_matrix.m[2][2]) / rect_h;
        c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];
    }
    else
    {
        /* Compute interpolation steps. */
        x_step[0] = inverse_matrix.m[0][0] / rect_w;
        x_step[1] = inverse_matrix.m[1][0] / rect_h;
        x_step[2] = inverse_matrix.m[2][0];
        y_step[0] = inverse_matrix.m[0][1] / rect_w;
        y_step[1] = inverse_matrix.m[1][1] / rect_h;
        y_step[2] = inverse_matrix.m[2][1];
        c_step[0] = (0.5f * (inverse_matrix.m[0][0] + inverse_matrix.m[0][1]) + inverse_matrix.m[0][2]) / rect_w;
        c_step[1] = (0.5f * (inverse_matrix.m[1][0] + inverse_matrix.m[1][1]) + inverse_matrix.m[1][2]) / rect_h;
        c_step[2] = 0.5f * (inverse_matrix.m[2][0] + inverse_matrix.m[2][1]) + inverse_matrix.m[2][2];
    }

    /* Determine image mode (NORMAL, NONE , MULTIPLY or STENCIL) depending on the color. */
    switch (source->image_mode) {
        case VG_LITE_NONE_IMAGE_MODE:
            imageMode = 0x00000001;
            break;
        case VG_LITE_MULTIPLY_IMAGE_MODE:
            imageMode = 0x00002001;
            break;
        case VG_LITE_NORMAL_IMAGE_MODE:
        case VG_LITE_ZERO:
            imageMode = 0x00001001;
            break;
        case VG_LITE_STENCIL_MODE:
            imageMode = 0x00003001;
            break;
        case VG_LITE_RECOLOR_MODE:
            imageMode = 0x00006001;
            break;
    }

    switch (filter) {
    case VG_LITE_FILTER_POINT:
        filter_mode = 0;
        break;

    case VG_LITE_FILTER_LINEAR:
        filter_mode = 0x10000;
        break;

    case VG_LITE_FILTER_BI_LINEAR:
        filter_mode = 0x20000;
        break;

    case VG_LITE_FILTER_GAUSSIAN:
        filter_mode = 0x30000;
        break;
    }

    in_premult = 0x10000000;
#if (CHIPID==0x555 || CHIPID==0x265)
    switch (source->format) {
        case VG_LITE_A4:
        case VG_LITE_A8:
            in_premult = 0x00000000;
            break;
        default:
            break;
    };
    switch (target->format) {
        case VG_LITE_RGB565:
        case VG_LITE_BGR565:
        case VG_LITE_RGB888:
        case VG_LITE_BGR888:
            in_premult = 0x00000000;
            break;
        default:
            break;
    };
#endif

#if !gcFEATURE_VG_SRC_PREMULTIPLIED
    if (forced_blending == VG_LITE_BLEND_PREMULTIPLY_SRC_OVER)
        in_premult = 0x00000000;
#endif

    blend_mode = convert_blend(forced_blending);
    tiled_source = (source->tiled != VG_LITE_LINEAR) ? 0x10000000 : 0 ;
    tiled = (target->tiled != VG_LITE_LINEAR) ? 0x40 : 0;
    if (blend_mode) {
        /* The hw bit for improve read image buffer performance when enable alpha blending. */
        ahb_read_split = 1 << 7;
    }
    compress_mode = (uint32_t)source->compress_mode << 25;
    if (source->compress_mode || target->compress_mode) {
        stripe_mode = 0x20000000;
    }

    /* Setup the command buffer. */
#if gcFEATURE_VG_GLOBAL_ALPHA
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AD1, s_context.dst_alpha_mode | s_context.dst_alpha_value | s_context.src_alpha_mode | s_context.src_alpha_value));
#endif
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A00, in_premult | imageMode | blend_mode | transparency_mode | tiled | s_context.enable_mask | s_context.matrix_enable | eco_fifo | s_context.scissor_enable | s_context.color_transform | stripe_mode));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A02, color));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A18, (void *) &c_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A19, (void *) &c_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1A, (void *) &c_step[2]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1C, (void *) &x_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1D, (void *) &x_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A1E, (void *) &x_step[2]));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A1F, 0x00000001));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A20, (void *) &y_step[0]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A21, (void *) &y_step[1]));
    VG_LITE_RETURN_ERROR(push_state_ptr(&s_context, 0x0A22, (void *) &y_step[2]));
    
    if (((source->format >= VG_LITE_YUY2) &&
         (source->format <= VG_LITE_AYUY2)) ||
        ((source->format >= VG_LITE_YUY2_TILED) &&
         (source->format <= VG_LITE_AYUY2_TILED))) {
            yuv2rgb = convert_yuv2rgb(source->yuv.yuv2rgb);
            uv_swiz = convert_uv_swizzle(source->yuv.swizzle);
    }

    if (((source->format >= VG_LITE_YUY2) &&
         (source->format <= VG_LITE_AYUY2)) ||
        ((source->format >= VG_LITE_YUY2_TILED) &&
         (source->format <= VG_LITE_AYUY2_TILED))) {
            yuv2rgb = convert_yuv2rgb(source->yuv.yuv2rgb);
            uv_swiz = convert_uv_swizzle(source->yuv.swizzle);
    }

#if gcFEATURE_VG_HW_PREMULTIPLY
    if (s_context.premultiply_src) {
        src_premultiply_enable = 0x01000100;
    } else {
        src_premultiply_enable = 0x01000000;
    }
#else
    src_premultiply_enable = 0x01000000;
#endif

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A25, convert_source_format(source->format) | filter_mode | uv_swiz | yuv2rgb | conversion | ahb_read_split | compress_mode | src_premultiply_enable | index_endian));
    if (source->yuv.uv_planar) {
        /* Program u plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A51, source->yuv.uv_planar));
    }
    if (source->yuv.v_planar) {
        /* Program v plane address if necessary. */
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A53, source->yuv.v_planar));
    }

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A27, 0));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A29, source->address));

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A34, 0));
    /* 24bit format stride configured to 4bpp. */
    if (source->format >= VG_LITE_RGB888 && source->format <= VG_LITE_RGBA5658) {
        stride = source->stride / 3 * 4;
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, stride | tiled_source));
    }
    else {
        VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2B, source->stride | tiled_source));
    }
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2D, rect_x | (rect_y << 16)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A2F, rect_w | (rect_h << 16)));
    VG_LITE_RETURN_ERROR(push_rectangle(&s_context, point_min.x, point_min.y, point_max.x - point_min.x,
                                        point_max.y - point_min.y));

#if gcFEATURE_VG_STRIPE_MODE
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0E02, 0x10 | (0x7 << 8)));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0F00, 0x10 | (0x7 << 8)));
#endif

    if (!s_context.flexa_mode)
        error = flush_target();
    vglitemDUMP_BUFFER("image", (size_t)source->address, source->memory, 0, (source->stride)*(source->height));
#if DUMP_IMAGE
    dump_img(source->memory, source->width, source->height, source->format);
#endif
    
    return error;
#else
    return VG_LITE_NOT_SUPPORT;
#endif
}

/* Program initial states for tessellation buffer. */
static vg_lite_error_t program_tessellation(vg_lite_context_t *context)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    uint32_t tessellation_size;

#if (CHIPID==0x355 || CHIPID==0x255)

    /* Compute tessellation buffer size. */
    uint32_t width = (context->tessbuf.tess_w_h & 0xFFFF);
    /* uint32_t height = (context->tessbuf.tess_w_h >> 16); */

    context->tessbuf.tess_stride = VG_LITE_ALIGN(width * 8, 64);

    /* Each bit in the L1 cache represents 64 bytes of tessellation data. */
    context->tessbuf.L1_size = VG_LITE_ALIGN(VG_LITE_ALIGN(context->tessbuf.tessbuf_size / 64, 64) / 8, 64);
#if (CHIPID==0x355)
    /* Each bit in the L2 cache represents 32 bytes of L1 data. */
    context->tessbuf.L2_size = VG_LITE_ALIGN(VG_LITE_ALIGN(context->tessbuf.L1_size / 32, 64) / 8, 64);
    tessellation_size = context->tessbuf.L2_size;
#else /* CHIPID: 0x255 */
    tessellation_size = context->tessbuf.L1_size;
#endif

    context->tessbuf.L1_phyaddr = context->tessbuf.physical_addr + context->tessbuf.tessbuf_size;
    context->tessbuf.L2_phyaddr = context->tessbuf.L1_phyaddr + context->tessbuf.L1_size;
    context->tessbuf.L1_logical = context->tessbuf.logical_addr + context->tessbuf.tessbuf_size;
    context->tessbuf.L2_logical = context->tessbuf.L1_logical + context->tessbuf.L1_size;

    /* Program tessellation buffer: input for VG module. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A30, context->tessbuf.physical_addr));   /* Tessellation buffer address. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A31, context->tessbuf.L1_phyaddr));   /* L1 address of tessellation buffer. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A32, context->tessbuf.L2_phyaddr));   /* L2 address of tessellation buffer. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A33, context->tessbuf.tess_stride));

    /* Program tessellation control: for TS module. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A35, context->tessbuf.physical_addr));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A36, context->tessbuf.L1_phyaddr));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A37, context->tessbuf.L2_phyaddr));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A38, context->tessbuf.tess_stride));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A3A, context->tessbuf.tess_w_h));

#if (REVISION==0x1217 && CID==0x407)
     VG_LITE_RETURN_ERROR(push_state(context, 0x0AB1, context->tessbuf.tess_stride));
     VG_LITE_RETURN_ERROR(push_state(context, 0x0AB2, context->tessbuf.tess_stride));
#endif

    VG_LITE_RETURN_ERROR(push_state(context, 0x0A3D, tessellation_size / 64));

#else /* (CHIPID==0x355 || CHIPID==0x255) */

    tessellation_size =  context->tessbuf.tessbuf_size;

    /* Program tessellation control: for TS module. */
    VG_LITE_RETURN_ERROR(push_state(context, 0x0A35, context->tessbuf.physical_addr));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0AC8, tessellation_size));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0ACB, context->tessbuf.physical_addr + tessellation_size));
    VG_LITE_RETURN_ERROR(push_state(context, 0x0ACC, context->tessbuf.countbuf_size));

#endif /* (CHIPID==0x355 || CHIPID==0x255) */

    return error;
}

vg_lite_error_t vg_lite_init(vg_lite_int32_t tess_width, vg_lite_int32_t tess_height)
{
    vg_lite_error_t error;
    vg_lite_kernel_initialize_t initialize;
    uint8_t i;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_init %d %d\n", tess_width, tess_height);
#endif

    s_context.rtbuffer = (vg_lite_buffer_t *)vg_lite_os_malloc(sizeof(vg_lite_buffer_t));
    if (!s_context.rtbuffer)
        return VG_LITE_OUT_OF_RESOURCES;
    memset(s_context.rtbuffer, 0, sizeof(vg_lite_buffer_t));

    if (tess_width <= 0) {
        tess_width = 0;
        tess_height = 0;
    }
    if (tess_height <= 0) {
        tess_height = 0;
        tess_width = 0;
    }
    tess_width  = VG_LITE_ALIGN(tess_width, 16);

    /* Allocate a command buffer and a tessellation buffer. */
    initialize.command_buffer_size = command_buffer_size;
    initialize.tess_width = tess_width;
    initialize.tess_height = tess_height;
    initialize.context = &s_context.context;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_INITIALIZE, &initialize));

    /* Verify driver ChipId/ChipRevision/Cid match hardware chip information */
    VG_LITE_RETURN_ERROR(check_hardware_chip_info());

    /* Save draw context. */
    s_context.capabilities = initialize.capabilities;
    s_context.command_buffer[0] = (uint8_t *)initialize.command_buffer[0];
    s_context.command_buffer[1] = (uint8_t *)initialize.command_buffer[1];
    s_context.command_buffer_size = initialize.command_buffer_size;
    s_context.command_offset[0] = 0;
    s_context.command_offset[1] = 0;

    if ((tess_width  > 0) && (tess_height > 0))
    {
        /* Set and Program Tessellation Buffer states. */
        s_context.tessbuf.physical_addr = initialize.physical_addr;
        s_context.tessbuf.logical_addr = initialize.logical_addr;
        s_context.tessbuf.tess_w_h = initialize.tess_w_h;
        s_context.tessbuf.tessbuf_size = initialize.tessbuf_size;
        s_context.tessbuf.countbuf_size = initialize.countbuf_size;

        VG_LITE_RETURN_ERROR(program_tessellation(&s_context));
        /* Init register gcregVGPEColorKey. */
        for (i = 0; i < 8; i++) {
            VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A90 + i, 0));
        }
    }

    s_context.custom_tessbuf = 0;
    s_context.custom_cmdbuf = 0;
    s_context.target_width = tess_width;
    s_context.target_height = tess_height;

    /* Init scissor rect. */
    s_context.scissor[0] =
    s_context.scissor[1] =
    s_context.scissor[2] =
    s_context.scissor[3] = 0;

    s_context.path_counter = 0;

    s_context.premultiply_dst = 1;

#if (CHIPID==0x355 || CHIPID==0x255)
    s_context.mirror_orient = VG_LITE_ORIENTATION_BOTTOM_TOP;
    s_context.premultiply_src = 0;
#else
    s_context.mirror_orient = VG_LITE_ORIENTATION_TOP_BOTTOM;
    s_context.premultiply_src = 1;
#endif

#if DUMP_CAPTURE
    _SetDumpFileInfo();
#endif

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_close(void)
{
    vg_lite_error_t error;
    vg_lite_kernel_terminate_t terminate;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_close\n");
#endif

    if (s_context.scissor_layer && s_context.scissor_layer->handle)
    {
        vg_lite_free(s_context.scissor_layer);
    }

    if (s_context.custom_cmdbuf)
    {
        vg_lite_kernel_unmap_memory_t unmap = {0};
        unmap.bytes = s_context.command_buffer_size * 2;
        unmap.logical = s_context.command_buffer[0];
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
    }

    if (s_context.custom_tessbuf)
    {
        vg_lite_kernel_unmap_memory_t unmap = {0};
        unmap.bytes = s_context.tessbuf.tessbuf_size + s_context.tessbuf.countbuf_size;
        unmap.logical = s_context.tessbuf.logical_addr;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
    }

    /* Termnate the draw context. */
    terminate.context = &s_context.context;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_TERMINATE, &terminate));

    if (s_context.rtbuffer)
        vg_lite_os_free(s_context.rtbuffer);

    submit_flag = 0;

    /* Reset the draw context. */
    memset(&s_context, 0, sizeof(s_context));

#if DUMP_CAPTURE
    _SetDumpFileInfo();
#endif
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_command_buffer_size(vg_lite_uint32_t size)
{
#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_set_command_buffer_size %d\n", size);
#endif

    if (command_buffer_size == 0)
        return VG_LITE_INVALID_ARGUMENT;

    command_buffer_size = size;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_command_buffer(vg_lite_uint32_t physical, vg_lite_uint32_t size)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_map_memory_t map = { 0 };

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_set_command_buffer 0x%08X %d\n", physical, size);
#endif

    if ((physical == 0) || (size == 0) || (physical % 64) || (size % 128))
        return VG_LITE_INVALID_ARGUMENT;

    map.bytes = size;
    map.physical = physical;

    if (s_context.command_buffer[0])
    {
        vg_lite_kernel_unmap_memory_t unmap = { 0 };
        if (submit_flag)
            VG_LITE_RETURN_ERROR(stall(&s_context, 0, (uint32_t)~0));
        if (!s_context.custom_cmdbuf)
        {
            vg_lite_kernel_free_t free;
            free.memory_handle = s_context.context.command_buffer[0];
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
            s_context.context.command_buffer[0] = 0;

            free.memory_handle = s_context.context.command_buffer[1];
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
            s_context.context.command_buffer[1] = 0;
        }
        unmap.bytes = s_context.command_buffer_size;
        unmap.logical = s_context.command_buffer[0];
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
        unmap.bytes = s_context.command_buffer_size;
        unmap.logical = s_context.command_buffer[1];
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
    }

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP_MEMORY, &map));

    s_context.context.command_buffer_logical[0] = map.logical;
    s_context.context.command_buffer_physical[0] = map.physical;

    s_context.context.command_buffer_logical[1] = (void*)((uint8_t*)map.logical + map.bytes / 2);
    s_context.context.command_buffer_physical[1] = map.physical + map.bytes / 2;

    s_context.command_buffer[0] = s_context.context.command_buffer_logical[0];
    s_context.command_buffer[1] = s_context.context.command_buffer_logical[1];
    s_context.command_offset[0] = 0;
    s_context.command_offset[1] = 0;
    s_context.command_buffer_current = 0;
    s_context.command_buffer_size = map.bytes / 2;
    s_context.custom_cmdbuf = 1;

    return error;
}

vg_lite_error_t vg_lite_set_tess_buffer(vg_lite_uint32_t physical, vg_lite_uint32_t size)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_map_memory_t map = { 0 };

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_set_tess_buffer 0x%08X %d\n", physical, size);
#endif

    if ((physical == 0) || (size == 0) || (physical % 64) || (size % 64) || (size < MIN_TS_SIZE))
        return VG_LITE_INVALID_ARGUMENT;

    map.bytes = size;
    map.physical = physical;

    if (s_context.tessbuf.logical_addr)
    {
        vg_lite_kernel_unmap_memory_t unmap = { 0 };
        if (submit_flag)
            VG_LITE_RETURN_ERROR(stall(&s_context, 0, (uint32_t)~0));
        if (!s_context.custom_tessbuf)
        {
            vg_lite_kernel_free_t free;
            free.memory_handle = s_context.context.tess_buffer;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));
            s_context.context.tess_buffer = 0;
        }
        unmap.bytes = s_context.tessbuf.tessbuf_size + s_context.tessbuf.countbuf_size;
        unmap.logical = s_context.tessbuf.logical_addr;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP_MEMORY, &unmap));
    }

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP_MEMORY, &map));

    s_context.tessbuf.logical_addr = map.logical;
    s_context.tessbuf.physical_addr = map.physical;
    s_context.tessbuf.countbuf_size = size * 3 / 128;
    s_context.tessbuf.countbuf_size = VG_LITE_ALIGN(s_context.tessbuf.countbuf_size, 64);
    s_context.tessbuf.tessbuf_size = map.bytes - s_context.tessbuf.countbuf_size;

    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0A35, s_context.tessbuf.physical_addr));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0AC8, s_context.tessbuf.tessbuf_size));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0ACB, s_context.tessbuf.physical_addr + s_context.tessbuf.tessbuf_size));
    VG_LITE_RETURN_ERROR(push_state(&s_context, 0x0ACC, s_context.tessbuf.countbuf_size));

    s_context.custom_tessbuf = 1;
    return error;
}

vg_lite_error_t vg_lite_get_mem_size(vg_lite_uint32_t* size)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_mem_t mem;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_get_mem_size %p\n", size);
#endif

    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_QUERY_MEM, &mem));
    *size = mem.bytes;

    return error;
}

/* Handle tiled & yuv allocation. Currently including NV12, ANV12, YV12, YV16, NV16, YV24. */
static  vg_lite_error_t _allocate_tiled_yuv_planar(vg_lite_buffer_t *buffer)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    uint32_t    yplane_size = 0;
    vg_lite_kernel_allocate_t allocate, uv_allocate, v_allocate;
    
    if ((buffer->format < VG_LITE_NV12) || (buffer->format > VG_LITE_ANV12_TILED)
        || (buffer->format == VG_LITE_AYUY2) || (buffer->format == VG_LITE_YUY2_TILED))
    {
        return error;
    }
    
    /* For NV12, there are 2 planes (Y, UV);
     For ANV12, there are 3 planes (Y, UV, Alpha).
     Each plane must be aligned by (4, 8).
     Then Y plane must be aligned by (8, 8).
     For YVxx, there are 3 planes (Y, U, V).
     YV12 is similar to NV12, both YUV420 format.
     YV16 and NV16 are YUV422 format.
     YV24 is YUV444 format.
     */
    buffer->width = VG_LITE_ALIGN(buffer->width, 8);
    buffer->height = VG_LITE_ALIGN(buffer->height, 8);
    buffer->stride = VG_LITE_ALIGN(buffer->width, 128);
    
    switch (buffer->format) {
        case VG_LITE_NV12:
        case VG_LITE_ANV12:
        case VG_LITE_NV12_TILED:
        case VG_LITE_ANV12_TILED:
            buffer->yuv.uv_stride = buffer->stride;
            buffer->yuv.alpha_stride = buffer->stride;
            buffer->yuv.uv_height = buffer->height / 2;
            break;
            
        case VG_LITE_NV16:
            buffer->yuv.uv_stride = buffer->stride;
            buffer->yuv.uv_height = buffer->height;
            break;
            
        case VG_LITE_YV12:
            buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride / 2;
            buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height / 2;
            break;
            
        case VG_LITE_YV16:
            buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride;
            buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height / 2;
            break;
            
        case VG_LITE_YV24:
            buffer->yuv.uv_stride =
            buffer->yuv.v_stride = buffer->stride;
            buffer->yuv.uv_height =
            buffer->yuv.v_height = buffer->height;
            break;
            
        default:
            return error;
    }
    
    yplane_size = buffer->stride * buffer->height;
    
    /* Allocate buffer memory: Y. */
    allocate.bytes = yplane_size;
    allocate.contiguous = 1;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &allocate));
    
    /* Save the allocation. */
    buffer->handle  = allocate.memory_handle;
    buffer->memory  = allocate.memory;
    buffer->address = allocate.memory_gpu;
    
    if ((buffer->format == VG_LITE_NV12) || (buffer->format == VG_LITE_ANV12)
        || (buffer->format == VG_LITE_NV16) || (buffer->format == VG_LITE_NV12_TILED)
        || (buffer->format == VG_LITE_ANV12_TILED)) {
        /* Allocate buffer memory: UV. */
        uv_allocate.bytes = buffer->yuv.uv_stride * buffer->yuv.uv_height;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &uv_allocate));
        buffer->yuv.uv_handle = uv_allocate.memory_handle;
        buffer->yuv.uv_memory = uv_allocate.memory;
        buffer->yuv.uv_planar = uv_allocate.memory_gpu;
        
        if ((buffer->format == VG_LITE_ANV12) || (buffer->format == VG_LITE_ANV12_TILED)) {
            uv_allocate.bytes = yplane_size;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &uv_allocate));
            buffer->yuv.alpha_planar = uv_allocate.memory_gpu;
        }
    } else {
        /* Allocate buffer memory: U, V. */
        uv_allocate.bytes = buffer->yuv.uv_stride * buffer->yuv.uv_height;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &uv_allocate));
        buffer->yuv.uv_handle = uv_allocate.memory_handle;
        buffer->yuv.uv_memory = uv_allocate.memory;
        buffer->yuv.uv_planar = uv_allocate.memory_gpu;
        
        v_allocate.bytes = buffer->yuv.v_stride * buffer->yuv.v_height;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &v_allocate));
        buffer->yuv.v_handle = v_allocate.memory_handle;
        buffer->yuv.v_memory = v_allocate.memory;
        buffer->yuv.v_planar = v_allocate.memory_gpu;
    }
    
    return error;
}

vg_lite_error_t vg_lite_allocate(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_kernel_allocate_t allocate;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_allocate %p\n", buffer);
#endif

    if (buffer->format == VG_LITE_RGBA8888_ETC2_EAC &&
       (buffer->width % 16 || buffer->height % 4)) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    /* Reset planar. */
    buffer->yuv.uv_planar =
    buffer->yuv.v_planar =
    buffer->yuv.alpha_planar = 0;

    /* Align height in case format is tiled. */
    if (buffer->format >= VG_LITE_YUY2 && buffer->format <= VG_LITE_NV16) {
        buffer->height = VG_LITE_ALIGN(buffer->height, 4);
        buffer->yuv.swizzle = VG_LITE_SWIZZLE_UV;
    }

    if (buffer->format >= VG_LITE_YUY2_TILED && buffer->format <= VG_LITE_AYUY2_TILED) {
        buffer->height = VG_LITE_ALIGN(buffer->height, 4);
        buffer->tiled = VG_LITE_TILED;
        buffer->yuv.swizzle = VG_LITE_SWIZZLE_UV;
    }

    if ((buffer->format >= VG_LITE_NV12 && buffer->format <= VG_LITE_ANV12_TILED
         && buffer->format != VG_LITE_AYUY2 && buffer->format != VG_LITE_YUY2_TILED)) {
        _allocate_tiled_yuv_planar(buffer);
    }
    else {
        /* Driver need compute the stride always with RT500 project. */

        vg_lite_float_t ratio = 1.0f;
        uint32_t mul, div, align;
        get_format_bytes(buffer->format, &mul, &div, &align);
        buffer->stride = buffer->width * mul / div;

#if gcFEATURE_VG_16PIXELS_ALIGNED
        buffer->stride = VG_LITE_ALIGN(buffer->stride, (16 * mul / div));
#endif

        /* Allocate the buffer. */
        if (buffer->compress_mode)
            ratio = _calc_decnano_compress_ratio(buffer->format, buffer->compress_mode);
        allocate.bytes = (uint32_t)(buffer->stride * buffer->height * ratio);

#if gcFEATURE_VG_IM_FASTCLEAR
        allocate.bytes = VG_LITE_ALIGN(allocate.bytes, 64);
#endif
        allocate.contiguous = 1;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &allocate));

        /* Save the buffer allocation. */
        buffer->handle  = allocate.memory_handle;
        buffer->memory  = allocate.memory;
        buffer->address = allocate.memory_gpu;

        if ((buffer->format == VG_LITE_AYUY2) || (buffer->format == VG_LITE_AYUY2_TILED) || ((buffer->format >= VG_LITE_ABGR8565_PLANAR)
             && (buffer->format <= VG_LITE_RGBA5658_PLANAR))) {
            allocate.bytes = buffer->stride * buffer->height;
            VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_ALLOCATE, &allocate));
            buffer->yuv.alpha_planar = allocate.memory_gpu;
        }

    }

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("=>buffer: width=%d, height=%d, stride=%d, bytes=%d, format=%d\n", 
        buffer->width, buffer->height, buffer->stride, allocate.bytes, buffer->format);
#endif

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_free(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error;
    vg_lite_kernel_free_t free, uv_free, v_free;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_free %p\n", buffer);
#endif

    if (buffer == NULL)
        return VG_LITE_INVALID_ARGUMENT;
    if (!(memcmp(s_context.rtbuffer,buffer,sizeof(vg_lite_buffer_t))) ) {
        if (VG_LITE_SUCCESS == submit(&s_context)) {
            VG_LITE_RETURN_ERROR(stall(&s_context, 0, ~0));
        }
        vglitemDUMP("@[swap 0x%08X %dx%d +%u]",
            s_context.rtbuffer->address,
            s_context.rtbuffer->width, s_context.rtbuffer->height,
            s_context.rtbuffer->stride);
        vglitemDUMP_BUFFER(
            "framebuffer",
            (size_t)s_context.rtbuffer->address,s_context.rtbuffer->memory,
            0,
            s_context.rtbuffer->stride*(s_context.rtbuffer->height));

        memset(s_context.rtbuffer, 0, sizeof(vg_lite_buffer_t));
    }

    if (buffer->yuv.uv_planar) {
        /* Free UV(U) planar buffer. */
        vglitemDUMP_BUFFER(
            "uv_plane",
            (size_t)buffer->yuv.uv_planar,buffer->yuv.uv_memory,
            0,
            buffer->yuv.uv_stride*buffer->yuv.uv_height);

        uv_free.memory_handle = buffer->yuv.uv_handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &uv_free));

        /* Mark the buffer as freed. */
        buffer->yuv.uv_handle = NULL;
        buffer->yuv.uv_memory = NULL;
    }

    if (buffer->yuv.v_planar) {
        /* Free V planar buffer. */
        vglitemDUMP_BUFFER(
            "v_plane",
            (size_t)buffer->yuv.v_planar,buffer->yuv.v_memory,
            0,
            buffer->yuv.v_stride*buffer->yuv.v_height);
        /* Free V planar buffer. */
        v_free.memory_handle = buffer->yuv.v_handle;
        VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &v_free));

        /* Mark the buffer as freed. */
        buffer->yuv.v_handle = NULL;
        buffer->yuv.v_memory = NULL;
    }

#if gcFEATURE_VG_IM_FASTCLEAR
    if (buffer->fc_buffer[0].handle != 0)
    {
#if VG_TARGET_FC_DUMP
        vglitemDUMP_BUFFER(
            "fcbuffer",
            (uint64_t)buffer->fc_buffer[0].address,buffer->fc_buffer[0].memory,
            0,
            buffer->fc_buffer[0].stride*(buffer->fc_buffer[0].height));
#endif
        _free_fc_buffer(&buffer->fc_buffer[0]);
    }
#endif 

    /* Make sure we have a valid memory handle. */
    if (buffer->handle == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    /* Free the buffer. */
    free.memory_handle = buffer->handle;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_FREE, &free));

    /* Mark the buffer as freed. */
    buffer->handle = NULL;
    buffer->memory = NULL;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_map(vg_lite_buffer_t* buffer, vg_lite_map_flag_t flag, int32_t fd)
{
    vg_lite_error_t error;
    vg_lite_kernel_map_t map;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_map %p\n", buffer);
#endif

    /* We either need a logical or physical address. */
    if (buffer->memory == NULL && buffer->address == 0) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    /* Check if we need to compute the stride. */
    if (buffer->stride == 0) {
        uint32_t mul, div, align;
        get_format_bytes(buffer->format, &mul, &div, &align);
        /* Compute the stride to be aligned. */
        buffer->stride = VG_LITE_ALIGN((buffer->width * mul / div), align);
    }

    /* Map the buffer. */
    map.bytes = buffer->stride * buffer->height;
    map.logical = buffer->memory;
    map.physical = buffer->address;

    if (flag == VG_LITE_MAP_USER_MEMORY) {
        map.flags = VG_LITE_HAL_MAP_USER_MEMORY;
    }
    else if (flag == VG_LITE_MAP_DMABUF) {
        map.flags = VG_LITE_HAL_MAP_DMABUF;
    }
    else {
        return VG_LITE_INVALID_ARGUMENT;
    }

    map.dma_buf_fd = fd;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_MAP, &map));

    /* Save the buffer allocation. */
    buffer->handle  = map.memory_handle;
    buffer->address = map.memory_gpu;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_unmap(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error;
    vg_lite_kernel_unmap_t unmap;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_unmap %p\n", buffer);
#endif

    /* Make sure we have a valid memory handle. */
    if (buffer->handle == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }
    
    /* Unmap the buffer. */
    unmap.memory_handle = buffer->handle;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_UNMAP, &unmap));
    
    /* Mark the buffer as freed. */
    buffer->handle = NULL;
    
    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_flush_mapped_buffer(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error;
    vg_lite_kernel_cache_t cache;

    /* Make sure we have a valid memory handle. */
    if (buffer->handle == NULL) {
        return VG_LITE_INVALID_ARGUMENT;
    }

    cache.memory_handle = buffer->handle;
    cache.cache_op = VG_LITE_CACHE_FLUSH;
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_CACHE, &cache));

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_get_register(vg_lite_uint32_t address, vg_lite_uint32_t* result)
{
    vg_lite_error_t error;
    vg_lite_kernel_info_t data;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_get_register 0x%08X %p\n", address, result);
#endif

    /* Get input register address. */
    data.addr = address;

    /* Get register info. */
    VG_LITE_RETURN_ERROR(vg_lite_kernel(VG_LITE_CHECK, &data));

    /* Return register info. */
    *result = data.reg;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_get_info(vg_lite_info_t *info)
{
#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_get_info %p\n", info);
#endif

    if (info != NULL)
    {
        info->api_version = VGLITE_API_VERSION_3_0;
        info->header_version = VGLITE_HEADER_VERSION;
        info->release_version = VGLITE_RELEASE_VERSION;
        info->reserved = 0;
    }

    return VG_LITE_SUCCESS;
}

vg_lite_uint32_t vg_lite_get_product_info(vg_lite_char* name, vg_lite_uint32_t* chip_id, vg_lite_uint32_t* chip_rev)
{
    const char *product_name;
    uint32_t name_len;
    uint32_t rev = 0, id = 0;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_get_product_info %p %p %p\n", name, chip_id, chip_rev);
#endif

    vg_lite_get_register(0x24, &rev);
    vg_lite_get_register(0x20, &id);

    if (id == 0x265 || id == 0x555)
        product_name = "GCNanoUltraV";
    else if (id == 0x255)
        product_name = "GCNanoLiteV";
    else if (id == 0x355)
        product_name = "GC355";
    else
        product_name = "Unknown";

    name_len = strlen(product_name) + 1;
    if (name != NULL)
    {
        memcpy(name, product_name, name_len);
    }
    
    if (chip_id != NULL)
    {
        *chip_id = id;
    }
    
    if (chip_rev != NULL)
    {
        *chip_rev = rev;
    }

    return name_len;
}

vg_lite_uint32_t vg_lite_query_feature(vg_lite_feature_t feature)
{
    uint32_t result;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_query_feature %d\n", feature);
#endif

    if (feature < gcFEATURE_COUNT)
        result = s_ftable.ftable[feature];
    else
        result = 0;

    return result;
}

vg_lite_error_t vg_lite_finish()
{
    vg_lite_error_t  error;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_finish\n");
#endif

    /* Return if there is nothing to submit. */
    if (CMDBUF_OFFSET(s_context) == 0)
    {
        if (submit_flag)
            VG_LITE_RETURN_ERROR(stall(&s_context, 0, (uint32_t)~0));
        return VG_LITE_SUCCESS;
    }

    /* Flush is moved from each draw to here. */
    VG_LITE_RETURN_ERROR(flush_target());
    VG_LITE_RETURN_ERROR(submit(&s_context));
    VG_LITE_RETURN_ERROR(stall(&s_context, 0, (uint32_t)~0));
    if (s_context.rtbuffer->memory) {
        kd_mpi_sys_mmz_flush_cache(0, s_context.rtbuffer->memory, s_context.rtbuffer->stride * s_context.rtbuffer->height);
    }

#if gcFEATURE_VG_IM_FASTCLEAR
#if VG_TARGET_FC_DUMP
    if (s_context.rtbuffer != NULL) {
        fc_buf_dump(s_context.rtbuffer, &s_context.fcBuffer);
    }
#endif
#endif

    CMDBUF_SWAP(s_context);
    /* Reset command buffer. */
    CMDBUF_OFFSET(s_context) = 0;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_flush(void)
{
    vg_lite_error_t error;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_flush\n");
#endif

    /* Return if there is nothing to submit. */
    if (CMDBUF_OFFSET(s_context) == 0)
        return VG_LITE_SUCCESS;

    /* Wait if GPU has not completed previous CMD buffer */
    if (submit_flag)
    {
        VG_LITE_RETURN_ERROR(stall(&s_context, 0, (uint32_t)~0));
    }

    /* Submit the current command buffer. */
    VG_LITE_RETURN_ERROR(flush_target());
    VG_LITE_RETURN_ERROR(submit(&s_context));
    CMDBUF_SWAP(s_context);
    /* Reset command buffer. */
    CMDBUF_OFFSET(s_context) = 0;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_init_grad(vg_lite_linear_gradient_t *grad)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_init_grad %p\n", grad);
#endif

    /* Set the member values according to driver defaults. */
    grad->image.width = VLC_GRADIENT_BUFFER_WIDTH;
    grad->image.height = 1;
    grad->image.stride = 0;
    grad->image.format = VG_LITE_BGRA8888;
    
    /* Allocate the image for gradient. */
    error = vg_lite_allocate(&grad->image);
    
    grad->count = 0;
    
    return error;
}

vg_lite_error_t vg_lite_set_linear_grad(vg_lite_ext_linear_gradient_t *grad,
                                 vg_lite_uint32_t count,
                                 vg_lite_color_ramp_t *color_ramp,
                                 vg_lite_linear_gradient_parameter_t linear_gradient,
                                 vg_lite_gradient_spreadmode_t spread_mode,
                                 vg_lite_uint8_t pre_multiplied)
{
    static vg_lite_color_ramp_t default_ramp[] =
    {
        {
            0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        },
        {
            1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        }
    };

    uint32_t i, trg_count;
    vg_lite_float_t prev_stop;
    vg_lite_color_ramp_t *src_ramp;
    vg_lite_color_ramp_t *src_ramp_last;
    vg_lite_color_ramp_t *trg_ramp;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_set_linear_grad %p %d %p (%f %f %f %f) %d %d\n", grad, count, color_ramp,
        linear_gradient.X0, linear_gradient.X1, linear_gradient.Y0, linear_gradient.Y1, spread_mode, pre_multiplied);
#endif

    /* Reset the count. */
    trg_count = 0;

    if ((linear_gradient.X0 == linear_gradient.X1) && (linear_gradient.Y0 == linear_gradient.Y1))
        return VG_LITE_INVALID_ARGUMENT;

    grad->linear_grad = linear_gradient;
    grad->pre_multiplied = pre_multiplied;
    grad->spread_mode = spread_mode;

    if (!count || count > VLC_MAX_COLOR_RAMP_STOPS || color_ramp == NULL)
        goto Empty_sequence_handler;

    for(i = 0; i < count;i++)
        grad->color_ramp[i] = color_ramp[i];
    grad->ramp_length = count;

    /* Determine the last source ramp. */
    src_ramp_last
        = grad->color_ramp
        + grad->ramp_length;

    /* Set the initial previous stop. */
    prev_stop = -1;

    /* Reset the count. */
    trg_count = 0;

    /* Walk through the source ramp. */
    for (
        src_ramp = grad->color_ramp, trg_ramp = grad->converted_ramp;
        (src_ramp < src_ramp_last) && (trg_count < VLC_MAX_COLOR_RAMP_STOPS + 2);
        src_ramp += 1
        )
    {
        /* Must be in increasing order. */
        if (src_ramp->stop < prev_stop)
        {
            /* Ignore the entire sequence. */
            trg_count = 0;
            break;
        }

        /* Update the previous stop value. */
        prev_stop = src_ramp->stop;

        /* Must be within [0..1] range. */
        if ((src_ramp->stop < 0.0f) || (src_ramp->stop > 1.0f))
        {
            /* Ignore. */
            continue;
        }

        /* Clamp color. */
        ClampColor(COLOR_FROM_RAMP(src_ramp),COLOR_FROM_RAMP(trg_ramp),0);

        /* First stop greater then zero? */
        if ((trg_count == 0) && (src_ramp->stop > 0.0f))
        {
            /* Force the first stop to 0.0f. */
            trg_ramp->stop = 0.0f;

            /* Replicate the entry. */
            trg_ramp[1] = *trg_ramp;
            trg_ramp[1].stop = src_ramp->stop;

            /* Advance. */
            trg_ramp  += 2;
            trg_count += 2;
        }
        else
        {
            /* Set the stop value. */
            trg_ramp->stop = src_ramp->stop;

            /* Advance. */
            trg_ramp  += 1;
            trg_count += 1;
        }
    }

    /* Empty sequence? */
    if (trg_count == 0)
    {
        memcpy(grad->converted_ramp, default_ramp, sizeof(default_ramp));
        grad->converted_length = sizeof(default_ramp) / 5;
    }
    else
    {
        /* The last stop must be at 1.0. */
        if (trg_ramp[-1].stop != 1.0f)
        {
            /* Replicate the last entry. */
            *trg_ramp = trg_ramp[-1];

            /* Force the last stop to 1.0f. */
            trg_ramp->stop = 1.0f;

            /* Update the final entry count. */
            trg_count += 1;
        }

        /* Set new length. */
        grad->converted_length = trg_count;
    }
    return VG_LITE_SUCCESS;

Empty_sequence_handler:
    memcpy(grad->converted_ramp, default_ramp, sizeof(default_ramp));
    grad->converted_length = sizeof(default_ramp) / 5;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_update_linear_grad(vg_lite_ext_linear_gradient_t *grad)
{
    uint32_t ramp_length;
    vg_lite_color_ramp_t *color_ramp;
    uint32_t common, stop;
    uint32_t i, width;
    uint8_t* bits;
    vg_lite_float_t x0,y0,x1,y1,length;
    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_update_linear_grad %p\n", grad);
#endif

    /* Get shortcuts to the color ramp. */
    ramp_length = grad->converted_length;
    color_ramp       = grad->converted_ramp;

    x0 = grad->linear_grad.X0;
    y0 = grad->linear_grad.Y0;
    x1 = grad->linear_grad.X1;
    y1 = grad->linear_grad.Y1;
    length = (vg_lite_float_t)sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));

    if (length <= 0)
        return VG_LITE_INVALID_ARGUMENT;
    /* Find the common denominator of the color ramp stops. */
    if (length < 1)
    {
        common = 1;
    }
    else
    {
        common = (uint32_t)length;
    }

    for (i = 0; i < ramp_length; ++i)
    {
        if (color_ramp[i].stop != 0.0f)
        {
            vg_lite_float_t mul  = common * color_ramp[i].stop;
            vg_lite_float_t frac = mul - (vg_lite_float_t) floor(mul);
            if (frac > 0.00013f)    /* Suppose error for zero is 0.00013 */
            {
                common = MAX(common, (uint32_t) (1.0f / frac + 0.5f));
            }
        }
    }

    /* Compute the width of the required color array. */
    width = common + 1;

    /* Allocate the color ramp surface. */
    memset(&grad->image, 0, sizeof(grad->image));
    grad->image.width = width;
    grad->image.height = 1;
    grad->image.stride = 0;
    grad->image.image_mode = VG_LITE_NONE_IMAGE_MODE;
    grad->image.format = VG_LITE_ABGR8888;

    /* Allocate the image for gradient. */
    VG_LITE_RETURN_ERROR(vg_lite_allocate(&grad->image));
    memset(grad->image.memory, 0, grad->image.stride * grad->image.height);
    width = common + 1;
    /* Set pointer to color array. */
    bits = (uint8_t *)grad->image.memory;

    /* Start filling the color array. */
    stop = 0;
    for (i = 0; i < width; ++i)
    {
        vg_lite_float_t gradient;
        vg_lite_float_t color[4];
        vg_lite_float_t color1[4];
        vg_lite_float_t color2[4];
        vg_lite_float_t weight;

        if (i == 241)
            i = 241;
        /* Compute gradient for current color array entry. */
        gradient = (vg_lite_float_t) i / (vg_lite_float_t) (width - 1);

        /* Find the entry in the color ramp that matches or exceeds this
        ** gradient. */
        while (gradient > color_ramp[stop].stop)
        {
            ++stop;
        }

        if (gradient == color_ramp[stop].stop)
        {
            /* Perfect match weight 1.0. */
            weight = 1.0f;

            /* Use color ramp color. */
            color1[3] = color_ramp[stop].alpha;
            color1[2] = color_ramp[stop].blue;
            color1[1] = color_ramp[stop].green;
            color1[0] = color_ramp[stop].red;

            color2[3] =
            color2[2] =
            color2[1] =
            color2[0] = 0.0f;
        }
        else
        {
            if(stop == 0){
                return VG_LITE_INVALID_ARGUMENT;
            }
            /* Compute weight. */
            weight = (color_ramp[stop].stop - gradient)
                    / (color_ramp[stop].stop - color_ramp[stop - 1].stop);

            /* Grab color ramp color of previous stop. */
            color1[3] = color_ramp[stop - 1].alpha;
            color1[2] = color_ramp[stop - 1].blue;
            color1[1] = color_ramp[stop - 1].green;
            color1[0] = color_ramp[stop - 1].red;

            /* Grab color ramp color of current stop. */
            color2[3] = color_ramp[stop].alpha;
            color2[2] = color_ramp[stop].blue;
            color2[1] = color_ramp[stop].green;
            color2[0] = color_ramp[stop].red;
        }

        if (grad->pre_multiplied)
        {
            /* Pre-multiply the first color. */
            color1[2] *= color1[3];
            color1[1] *= color1[3];
            color1[0] *= color1[3];

            /* Pre-multiply the second color. */
            color2[2] *= color2[3];
            color2[1] *= color2[3];
            color2[0] *= color2[3];
        }

        /* Filter the colors per channel. */
        color[3] = LERP(color1[3], color2[3], weight);
        color[2] = LERP(color1[2], color2[2], weight);
        color[1] = LERP(color1[1], color2[1], weight);
        color[0] = LERP(color1[0], color2[0], weight);

        /* Pack the final color. */
        *bits++ = PackColorComponent(color[3]);
        *bits++ = PackColorComponent(color[2]);
        *bits++ = PackColorComponent(color[1]);
        *bits++ = PackColorComponent(color[0]);
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_radial_grad(vg_lite_radial_gradient_t *grad,
                                 vg_lite_uint32_t count,
                                 vg_lite_color_ramp_t *color_ramp,
                                 vg_lite_radial_gradient_parameter_t radial_grad,
                                 vg_lite_gradient_spreadmode_t spread_mode,
                                 vg_lite_uint8_t pre_multiplied)
{
    static vg_lite_color_ramp_t defaultRamp[] =
    {
        {
            0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        },
        {
            1.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        }
    };

    uint32_t i, trgCount;
    vg_lite_float_t prevStop;
    vg_lite_color_ramp_t *srcRamp;
    vg_lite_color_ramp_t *srcRampLast;
    vg_lite_color_ramp_t *trgRamp;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_set_radial_grad %p %d %p (%f %f %f %f %f) %d %d\n", grad, count, color_ramp,
        radial_grad.cx, radial_grad.cy, radial_grad.fx, radial_grad.fy, radial_grad.r, spread_mode, pre_multiplied);
#endif

    /* Reset the count. */
    trgCount = 0;

    if (radial_grad.r <= 0)
        return VG_LITE_INVALID_ARGUMENT;

    grad->radial_grad = radial_grad;
    grad->pre_multiplied = pre_multiplied;
    grad->spread_mode = spread_mode;

    if (!count || count > VLC_MAX_COLOR_RAMP_STOPS || color_ramp == NULL)
        goto Empty_sequence_handler;

    for(i = 0; i < count;i++)
        grad->color_ramp[i] = color_ramp[i];
    grad->ramp_length = count;

    /* Determine the last source ramp. */
    srcRampLast
        = grad->color_ramp
        + grad->ramp_length;

    /* Set the initial previous stop. */
    prevStop = -1;

    /* Reset the count. */
    trgCount = 0;

    /* Walk through the source ramp. */
    for (
        srcRamp = grad->color_ramp, trgRamp = grad->converted_ramp;
        (srcRamp < srcRampLast) && (trgCount < VLC_MAX_COLOR_RAMP_STOPS + 2);
        srcRamp += 1
        )
    {
        /* Must be in increasing order. */
        if (srcRamp->stop < prevStop)
        {
            /* Ignore the entire sequence. */
            trgCount = 0;
            break;
        }

        /* Update the previous stop value. */
        prevStop = srcRamp->stop;

        /* Must be within [0..1] range. */
        if ((srcRamp->stop < 0.0f) || (srcRamp->stop > 1.0f))
        {
            /* Ignore. */
            continue;
        }

        /* Clamp color. */
        ClampColor(COLOR_FROM_RAMP(srcRamp),COLOR_FROM_RAMP(trgRamp),0);

        /* First stop greater then zero? */
        if ((trgCount == 0) && (srcRamp->stop > 0.0f))
        {
            /* Force the first stop to 0.0f. */
            trgRamp->stop = 0.0f;

            /* Replicate the entry. */
            trgRamp[1] = *trgRamp;
            trgRamp[1].stop = srcRamp->stop;

            /* Advance. */
            trgRamp  += 2;
            trgCount += 2;
        }
        else
        {
            /* Set the stop value. */
            trgRamp->stop = srcRamp->stop;

            /* Advance. */
            trgRamp  += 1;
            trgCount += 1;
        }
    }

    /* Empty sequence? */
    if (trgCount == 0)
    {
        memcpy(grad->converted_ramp,defaultRamp,sizeof(defaultRamp));
        grad->converted_length = sizeof(defaultRamp) / 5;
    }
    else
    {
        /* The last stop must be at 1.0. */
        if (trgRamp[-1].stop != 1.0f)
        {
            /* Replicate the last entry. */
            *trgRamp = trgRamp[-1];

            /* Force the last stop to 1.0f. */
            trgRamp->stop = 1.0f;

            /* Update the final entry count. */
            trgCount += 1;
        }

        /* Set new length. */
        grad->converted_length = trgCount;
    }
    return VG_LITE_SUCCESS;

Empty_sequence_handler:
    memcpy(grad->converted_ramp,defaultRamp,sizeof(defaultRamp));
    grad->converted_length = sizeof(defaultRamp) / 5;

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_update_radial_grad(vg_lite_radial_gradient_t *grad)
{
    uint32_t ramp_length;
    vg_lite_color_ramp_t *colorRamp;
    uint32_t common, stop;
    uint32_t i, width;
    uint8_t* bits;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    uint32_t align, mul, div;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_update_radial_grad %p\n", grad);
#endif

    /* Get shortcuts to the color ramp. */
    ramp_length = grad->converted_length;
    colorRamp   = grad->converted_ramp;

    if (grad->radial_grad.r <= 0)
        return VG_LITE_INVALID_ARGUMENT;

    /* Find the common denominator of the color ramp stops. */
    if (grad->radial_grad.r < 1)
    {
        common = 1;
    }
    else
    {
        common = (uint32_t)grad->radial_grad.r;
    }

    for (i = 0; i < ramp_length; ++i)
    {
        if (colorRamp[i].stop != 0.0f)
        {
            vg_lite_float_t mul  = common * colorRamp[i].stop;
            vg_lite_float_t frac = mul - (vg_lite_float_t) floor(mul);
            if (frac > 0.00013f)    /* Suppose error for zero is 0.00013 */
            {
                common = MAX(common, (uint32_t) (1.0f / frac + 0.5f));
            }
        }
    }

    /* Compute the width of the required color array. */
    width = common + 1;
    width = (width + 15) & (~0xf);

    /* Allocate the color ramp surface. */
    memset(&grad->image, 0, sizeof(grad->image));
    grad->image.width = width;
    grad->image.height = 1;
    grad->image.stride = 0;
    grad->image.image_mode = VG_LITE_NONE_IMAGE_MODE;
    grad->image.format = VG_LITE_ABGR8888;

    /* Allocate the image for gradient. */
    VG_LITE_RETURN_ERROR(vg_lite_allocate(&grad->image));

    get_format_bytes(VG_LITE_ABGR8888, &mul, &div, &align);
    width = grad->image.stride * div / mul;

    /* Set pointer to color array. */
    bits = (uint8_t *)grad->image.memory;

    /* Start filling the color array. */
    stop = 0;
    for (i = 0; i < width; ++i)
    {
        vg_lite_float_t gradient;
        vg_lite_float_t color[4];
        vg_lite_float_t color1[4];
        vg_lite_float_t color2[4];
        vg_lite_float_t weight;

        /* Compute gradient for current color array entry. */
        gradient = (vg_lite_float_t) i / (vg_lite_float_t) (width - 1);

        /* Find the entry in the color ramp that matches or exceeds this
        ** gradient. */
        while (gradient > colorRamp[stop].stop)
        {
            ++stop;
        }

        if (gradient == colorRamp[stop].stop)
        {
            /* Perfect match weight 1.0. */
            weight = 1.0f;

            /* Use color ramp color. */
            color1[3] = colorRamp[stop].alpha;
            color1[2] = colorRamp[stop].blue;
            color1[1] = colorRamp[stop].green;
            color1[0] = colorRamp[stop].red;

            color2[3] =
            color2[2] =
            color2[1] =
            color2[0] = 0.0f;
        }
        else
        {
            /* Compute weight. */
            weight = (colorRamp[stop].stop - gradient)
                    / (colorRamp[stop].stop - colorRamp[stop - 1].stop);

            /* Grab color ramp color of previous stop. */
            color1[3] = colorRamp[stop - 1].alpha;
            color1[2] = colorRamp[stop - 1].blue;
            color1[1] = colorRamp[stop - 1].green;
            color1[0] = colorRamp[stop - 1].red;

            /* Grab color ramp color of current stop. */
            color2[3] = colorRamp[stop].alpha;
            color2[2] = colorRamp[stop].blue;
            color2[1] = colorRamp[stop].green;
            color2[0] = colorRamp[stop].red;
        }

        if (grad->pre_multiplied)
        {
            /* Pre-multiply the first color. */
            color1[2] *= color1[3];
            color1[1] *= color1[3];
            color1[0] *= color1[3];

            /* Pre-multiply the second color. */
            color2[2] *= color2[3];
            color2[1] *= color2[3];
            color2[0] *= color2[3];
        }

        /* Filter the colors per channel. */
        color[3] = LERP(color1[3], color2[3], weight);
        color[2] = LERP(color1[2], color2[2], weight);
        color[1] = LERP(color1[1], color2[1], weight);
        color[0] = LERP(color1[0], color2[0], weight);

        /* Pack the final color. */
        *bits++ = PackColorComponent(color[3]);
        *bits++ = PackColorComponent(color[2]);
        *bits++ = PackColorComponent(color[1]);
        *bits++ = PackColorComponent(color[0]);
    }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_set_grad(vg_lite_linear_gradient_t *grad,
                                 vg_lite_uint32_t count,
                                 vg_lite_uint32_t *colors,
                                 vg_lite_uint32_t *stops)
{
    uint32_t i;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_set_grad %p %d %p %p\n", grad, count, colors, stops);
#endif

    grad->count = 0;    /* Opaque B&W gradient */
    if (!count || count > VLC_MAX_GRADIENT_STOPS || colors == NULL || stops == NULL)
        return VG_LITE_SUCCESS;

    /* Check stops validity */
    for (i = 0; i < count; i++)
        if (stops[i] < VLC_GRADIENT_BUFFER_WIDTH) {
            if (!grad->count || stops[i] > grad->stops[grad->count - 1]) {
                grad->stops[grad->count] = stops[i];
                grad->colors[grad->count] = colors[i];
                grad->count++;
            } else if (stops[i] == grad->stops[grad->count - 1]) {
                /* Equal stops : use the color corresponding to the last stop
                in the sequence */
                grad->colors[grad->count - 1] = colors[i];
            }
        }

    return VG_LITE_SUCCESS;
}

vg_lite_error_t vg_lite_update_grad(vg_lite_linear_gradient_t *grad)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int32_t r0, g0, b0, a0;
    int32_t r1, g1, b1, a1;
    int32_t lr, lg, lb, la;
    uint32_t i;
    int32_t j;
    int32_t ds, dr, dg, db, da;
    uint32_t *buffer = (uint32_t *)grad->image.memory;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_update_grad %p\n", grad);
#endif

    if (grad->count == 0) {
        /* If no valid stops have been specified (e.g., due to an empty input
        * array, out-of-range, or out-of-order stops), a stop at 0 with color
        * 0xFF000000 (opaque black) and a stop at 255 with color 0xFFFFFFFF
        * (opaque white) are implicitly defined. */
        grad->stops[0] = 0;
        grad->colors[0] = 0xFF000000;   /* Opaque black */
        grad->stops[1] = 255;
        grad->colors[1] = 0xFFFFFFFF;   /* Opaque white */
        grad->count = 2;
    } else if (grad->count && grad->stops[0] != 0) {
        /* If at least one valid stop has been specified, but none has been
        * defined with an offset of 0, an implicit stop is added with an
        * offset of 0 and the same color as the first user-defined stop. */
        for (i = 0; i < grad->stops[0]; i++)
            buffer[i] = grad->colors[0];
    }
    a0 = A(grad->colors[0]);
    r0 = R(grad->colors[0]);
    g0 = G(grad->colors[0]);
    b0 = B(grad->colors[0]);

    /* Calculate the colors for each pixel of the image. */
    for (i = 0; i < grad->count - 1; i++) {
        buffer[grad->stops[i]] = grad->colors[i];
        ds = grad->stops[i + 1] - grad->stops[i];
        a1 = A(grad->colors[i + 1]);
        r1 = R(grad->colors[i + 1]);
        g1 = G(grad->colors[i + 1]);
        b1 = B(grad->colors[i + 1]);

        da = a1 - a0;
        dr = r1 - r0;
        dg = g1 - g0;
        db = b1 - b0;

        for (j = 1; j < ds; j++) {
            la = a0 + da * j / ds;
            lr = r0 + dr * j / ds;
            lg = g0 + dg * j / ds;
            lb = b0 + db * j / ds;

            buffer[grad->stops[i] + j] = ARGB(la, lr, lg, lb);
        }

        a0 = a1;
        r0 = r1;
        g0 = g1;
        b0 = b1;
    }

    /* If at least one valid stop has been specified, but none has been defined
    * with an offset of 255, an implicit stop is added with an offset of 255
    * and the same color as the last user-defined stop. */
    for (i = grad->stops[grad->count - 1]; i < VLC_GRADIENT_BUFFER_WIDTH; i++)
        buffer[i] = grad->colors[grad->count - 1];

    return error;
}

vg_lite_error_t vg_lite_clear_linear_grad(vg_lite_ext_linear_gradient_t *grad)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_clear_linear_grad %p\n", grad);
#endif

    grad->count = 0;
    /* Release the image resource. */
    if (grad->image.handle != NULL)
    {
        error = vg_lite_free(&grad->image);
    }

    return error;
}

vg_lite_error_t vg_lite_clear_grad(vg_lite_linear_gradient_t *grad)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_clear_grad %p\n", grad);
#endif

    grad->count = 0;
    /* Release the image resource. */
    if (grad->image.handle != NULL)
    {
        error = vg_lite_free(&grad->image);
    }
    
    return error;
}

vg_lite_error_t vg_lite_clear_radial_grad(vg_lite_radial_gradient_t *grad)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_clear_radial_grad %p\n", grad);
#endif

    grad->count = 0;
    /* Release the image resource. */
    if (grad->image.handle != NULL)
    {
        error = vg_lite_free(&grad->image);
    }

    return error;
}

vg_lite_matrix_t * vg_lite_get_linear_grad_matrix(vg_lite_ext_linear_gradient_t *grad)
{
#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_get_linear_grad_matrix %p\n", grad);
#endif

    return &grad->matrix;
}

vg_lite_matrix_t * vg_lite_get_grad_matrix(vg_lite_linear_gradient_t *grad)
{
#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_get_grad_matrix %p\n", grad);
#endif

    return &grad->matrix;
}

vg_lite_matrix_t * vg_lite_get_radial_grad_matrix(vg_lite_radial_gradient_t *grad)
{
#if gcFEATURE_VG_TRACE_API
    VGLITE_LOG("vg_lite_get_radial_grad_matrix %p\n", grad);
#endif

    return &grad->matrix;
}
