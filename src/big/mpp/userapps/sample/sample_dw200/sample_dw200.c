/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dw200.h"
#include "DewarpMap.h"
#include "mpi_dewarp_api.h"
#include "mpi_sys_api.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "cJSON.h"
#include <stdint.h>

#define COLOR_NONE "\033[0m"
#define RED "\033[1;31;40m"
#define BLUE "\033[1;34;40m"
#define GREEN "\033[1;32;40m"
#define YELLOW "\033[1;33;40m"

#define LOG_LEVEL 3

#define pr_info(fmt, ...) if(LOG_LEVEL>=2)fprintf(stderr,GREEN fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#define pr_warn(fmt, ...) if(LOG_LEVEL>=1)fprintf(stderr,YELLOW fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#define pr_err(fmt, ...) if(LOG_LEVEL>=0)fprintf(stderr,RED fmt "\n" COLOR_NONE, ##__VA_ARGS__)

// FIXME: dwe/vse use diff format
const char* format_table[] = {
    "YUV422SP",
    "YUV422I",
    "YUV420SP",
    "YUV444",
    "RGB888",
    "RGB888P",
    "RAW8",
    "RAW12"
};

struct vbuffer {
    k_u64 phy_addr;
    void* virt_addr;
    uint32_t size;
};

enum format_t get_format(const char* format) {
    for (unsigned i = 0; i < sizeof(format_table) / sizeof(char*); i++) {
        if (strcmp(format, format_table[i]) == 0) {
            return i;
        }
    }
    // default value
    return 2;
}

#define noi(v,d) (v ? v->valueint : d)
#define cj(o,s) cJSON_GetObjectItem(o,s)
#define jcj(s) cj(json,s)
#define ncj(s) cj(node,s)
#define joi(s,d) noi(ncj(s),d)
#define SetIfNotNull(s,v) if(ncj(v))s=ncj(v)->valueint
#define SetIfNotNullJ(s,v) if(jcj(v))s=jcj(v)->valueint

void parse_output(cJSON* node, int id, struct dw200_parameters* params) {
    if (node == NULL) {
        return;
    }
    SetIfNotNull(params->output_res[id].enable, "enabled");
    SetIfNotNull(params->output_res[id].width, "width");
    params->output_res[id].width = joi("width", 0);
    params->output_res[id].height = joi("height", 0);
    params->output_res[id].format = get_format(ncj("format")->valuestring);
    params->output_res[id].yuvbit = joi("yuvbit", 0);
    if (!params->output_res[id].enable) {
        return;
    }

    if (id > 0) {
        params->mi_settings[id - 1].width = params->output_res[id].width;
        params->mi_settings[id - 1].height = params->output_res[id].height;
        params->vse_enableResizeOutput[id - 1] = TRUE;
        params->mi_settings[id - 1].out_format = params->output_res[id].format;
        params->mi_settings[id - 1].yuvbit = params->output_res[id].yuvbit;
        params->mi_settings[id - 1].enable = 1;
        params->vse_format_conv[id - 1].out_format = params->output_res[id].format;

        cJSON* crop = ncj("crop");
        if (crop && (crop->type == cJSON_Array)) {
            params->vse_cropSize[id - 1].left = cJSON_GetArrayItem(crop, 0)->valueint;
            params->vse_cropSize[id - 1].right = cJSON_GetArrayItem(crop, 1)->valueint;
            params->vse_cropSize[id - 1].top = cJSON_GetArrayItem(crop, 2)->valueint;
            params->vse_cropSize[id - 1].bottom = cJSON_GetArrayItem(crop, 3)->valueint;
        }
    }
}

int load_user_map(const char* filename, uint32_t* map_buffer) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        perror(RED"open file error");
        return -1;
    }
    char sz[1024] = {0};
    uint32_t val = 0;
    unsigned ptr = 0;
    while (fgets(sz, 1024, f)) {
        sscanf(sz, "%08x", &val);
        map_buffer[ptr++] = val;
    }
    fclose(f);
    return ptr;
}

int parse(const char* config,
    char* input_image_file,
    char* input_image_file2,
    char* usermap,
    struct dewarp_distortion_map* dmap,
    struct dw200_parameters* params, uint64_t* goldenHashKey
) {
    FILE* f = fopen(config, "r");
    if (f == NULL) {
        perror("open config");
        return -1;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, f);
    fclose(f);
    buffer[size] = '\0';

    pr_info("config file size: %lu", size);

    cJSON* json = cJSON_Parse(buffer);
    if (!json) {
        pr_err("parse error");
        free(buffer);
        return -1;
    }
    cJSON_Print(json);

    // input 0
    {
        cJSON* input = jcj("input 0");
        if (input) {
            cJSON* input_file = cJSON_GetObjectItem(input, "file");
            if (input_file) {
                memcpy(input_image_file, input_file->valuestring, strlen(input_file->valuestring) + 1);
            } else {
                input_image_file[0] = '\0';
            }
            char* format = cJSON_GetObjectItem(input, "format")->valuestring;
            params->input_res[0].format = get_format(format);
            params->input_res[0].width = cJSON_GetObjectItem(input, "width")->valueint;
            params->input_res[0].height = cJSON_GetObjectItem(input, "height")->valueint;
            params->input_res[0].enable = cJSON_GetObjectItem(input, "enabled")->valueint;
            params->input_res[0].yuvbit = cJSON_GetObjectItem(input, "yuvbit")->valueint;
        } else {
            *input_image_file = '\0';
        }
    }

    // input 1
    {
        cJSON* input = jcj("input 1");
        if (input) {
            cJSON* input_file = cJSON_GetObjectItem(input, "file");
            if (input_file) {
                memcpy(input_image_file2, input_file->valuestring, strlen(input_file->valuestring) + 1);
            } else {
                input_image_file2[0] = '\0';
            }
            char* format = cJSON_GetObjectItem(input, "format")->valuestring;
            params->input_res[1].format = get_format(format);
            params->input_res[1].width = cJSON_GetObjectItem(input, "width")->valueint;
            params->input_res[1].height = cJSON_GetObjectItem(input, "height")->valueint;
            params->input_res[1].enable = cJSON_GetObjectItem(input, "enabled")->valueint;
            params->input_res[1].yuvbit = cJSON_GetObjectItem(input, "yuvbit")->valueint;
            params->vse_inputSelect = cJSON_GetObjectItem(input, "channel")->valueint;
        } else {
            *input_image_file2 = '\0';
        }
    }

    // golden hash key
    {
        cJSON* golden = jcj("goldenHashKey");
        if (golden && (golden->type == cJSON_String) && (golden->valuestring)) {
            char* endptr;
            *goldenHashKey = strtoull(golden->valuestring, &endptr, 10);
        }
    }

    // user map
    {
        cJSON* um = jcj("userMap");
        if (um && (um->type == cJSON_String) && (um->valuestring)) {
            memcpy(usermap, um->valuestring, strlen(um->valuestring) + 1);
            // TODO: load user map
        } else {
            *usermap = '\0';
            dmap->userMapSize = 0;
        }
    }

    // output
    {
        parse_output(jcj("output 0"), 0, params);
        parse_output(jcj("output 1"), 1, params);
        parse_output(jcj("output 2"), 2, params);
        parse_output(jcj("output 3"), 3, params);
    }

    // params
    params->dewarp_type = 2;
    params->scale_factor = 1.0 * 4096;
    {
        const char* dewarp_mode_table[] = {
            "LENS_CORRECTION",
            "FISHEYE_EXPAND",
            "SPLIT_SCREEN",
            "FISHEYE_DEWARP",
            "PERSPECTIVE",
        };
        cJSON* dm = jcj("dewarpMode");
        if (dm && (dm->type == cJSON_String) && (dm->valuestring)) {
            for (unsigned i = 0; i < sizeof(dewarp_mode_table) / sizeof(char*); i++) {
                if (strcmp(dm->valuestring, dewarp_mode_table[i]) == 0) {
                    params->dewarp_type = 1 << i;
                }
            }
        }
    }
    {
        cJSON* node = jcj("scale");
        if (node && (node->type == cJSON_Object)) {
            SetIfNotNull(params->roi_start.width, "roix");
            SetIfNotNull(params->roi_start.height, "roiy");
            if (ncj("factor")) {
                params->scale_factor = ncj("factor")->valuedouble * 4096;
            }
        }
    }
    {
        cJSON* node = jcj("split");
        if (node && (node->type == cJSON_Object)) {
            SetIfNotNull(params->split_horizon_line, "horizon_line");
            SetIfNotNull(params->split_vertical_line_up, "vertical_line_up");
            SetIfNotNull(params->split_vertical_line_down, "vertical_line_down");
        }
    }
    {
        SetIfNotNullJ(params->hflip, "hflip");
        SetIfNotNullJ(params->vflip, "vflip");
        SetIfNotNullJ(params->rotation, "rotation");
        SetIfNotNullJ(params->bypass, "bypass");
    }
    {
        cJSON* node = jcj("camera_matrix");
        if (node && (node->type == cJSON_Array)) {
            for (unsigned i = 0; i < cJSON_GetArraySize(node); i++) {
                dmap->camera_matrix[i] = cJSON_GetArrayItem(node, i)->valuedouble;
            }
        }
    }
    {
        cJSON* node = jcj("distortion_coeff");
        if (node && (node->type == cJSON_Array)) {
            for (unsigned i = 0; i < cJSON_GetArraySize(node); i++) {
                dmap->distortion_coeff[i] = cJSON_GetArrayItem(node, i)->valuedouble;
            }
        }
    }
    {
        cJSON* node = jcj("perspective");
        if (node && (node->type == cJSON_Array)) {
            for (unsigned i = 0; i < cJSON_GetArraySize(node); i++) {
                dmap->perspective_matrix[i] = cJSON_GetArrayItem(node, i)->valuedouble;
            }
        }
    }
    {
        cJSON* node = jcj("fov");
        SetIfNotNull(params->fov.offAngleUL, "offAngleUL");
        SetIfNotNull(params->fov.offAngleUR, "offAngleUR");
        SetIfNotNull(params->fov.offAngleDL, "offAngleDL");
        SetIfNotNull(params->fov.offAngleDR, "offAngleDR");
        SetIfNotNull(params->fov.fovUL, "fovUL");
        SetIfNotNull(params->fov.fovUR, "fovUR");
        SetIfNotNull(params->fov.fovDL, "fovDL");
        SetIfNotNull(params->fov.fovDR, "fovDR");
        SetIfNotNull(params->fov.panoAtWin, "panoAtWin");

        SetIfNotNull(params->fov.centerOffsetRatioUL, "centerOffsetRatioUL");
        SetIfNotNull(params->fov.centerOffsetRatioUR, "centerOffsetRatioUR");
        SetIfNotNull(params->fov.centerOffsetRatioDL, "centerOffsetRatioDL");
        SetIfNotNull(params->fov.centerOffsetRatioDR, "centerOffsetRatioDR");
        SetIfNotNull(params->fov.circleOffsetRatioUL, "circleOffsetRatioUL");
        SetIfNotNull(params->fov.circleOffsetRatioUR, "circleOffsetRatioUR");
        SetIfNotNull(params->fov.circleOffsetRatioDL, "circleOffsetRatioDL");
        SetIfNotNull(params->fov.circleOffsetRatioDR, "circleOffsetRatioDR");
    }
    params->boundary_pixel.Y = 0;
    params->boundary_pixel.U = 128;
    params->boundary_pixel.V = 128;

    cJSON_Delete(json);
    free(buffer);
    return 0;
#undef noi
#undef cj
#undef jcj
#undef ncj
#undef joi
}

static unsigned format_bpp(enum format_t format) {
    const unsigned table[] = {
    16, 16,
    12, 24,
    24, 24,
    8, 16,
    // unused
    0, 0, 0, 0
    };
    return table[format];
}

uint32_t resolution_size(const struct dw200_resolution* res) {
    return format_bpp(res->format) * res->width * res->height / 8;
}

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#define VS_PI   3.1415926535897932384626433832795
#define DEWARP_BUFFER_ALIGNMENT 16
enum {
    DEWARP_MODEL_LENS_DISTORTION_CORRECTION = 1 << 0,
    DEWARP_MODEL_FISHEYE_EXPAND             = 1 << 1,
    DEWARP_MODEL_SPLIT_SCREEN               = 1 << 2,
    DEWARP_MODEL_FISHEYE_DEWARP             = 1 << 3,
    DEWARP_MODEL_PERSPECTIVE                = 1 << 4,
};

void set_params(const struct dw200_parameters* params, struct k_dwe_hw_info* dwe_info, struct k_vse_params* vse_info) {
    const uint32_t blocksize = 16;
    const uint32_t blockshift = 4;
    const uint32_t MAX_IMG_WIDTH = 4096;
    dwe_info->src_w = params->input_res[0].width;
    dwe_info->src_h = params->input_res[0].height;
    dwe_info->in_yuvbit = params->input_res[0].yuvbit;
    dwe_info->out_yuvbit = params->output_res[0].yuvbit;
    dwe_info->roi_x = params->roi_start.width;
    dwe_info->roi_y = params->roi_start.height;
    dwe_info->map_w = (ALIGN_UP(dwe_info->src_w, blocksize) >> blockshift) + 1;
    dwe_info->map_h = (ALIGN_UP(dwe_info->src_h, blocksize) >> blockshift) + 1;
    if (params->dewarp_type == DEWARP_MODEL_SPLIT_SCREEN) {
        if (params->split_horizon_line > dwe_info->src_h &&
            params->split_vertical_line_up > dwe_info->src_w &&
            params->split_vertical_line_down > dwe_info->src_w) {
            // one screen
            dwe_info->dst_w = MIN(MAX_IMG_WIDTH, ALIGN_UP((uint32_t)(dwe_info->src_h * VS_PI) , 16));
            dwe_info->dst_h = ALIGN_UP(dwe_info->src_h / 2 , 8);
        } else if ((params->split_vertical_line_up > dwe_info->src_w ||
                 params->split_vertical_line_down > dwe_info->src_w) &&
                (params->split_horizon_line > blocksize && params->split_horizon_line < dwe_info->src_h)) {
            // up and down two screen or three
            dwe_info->dst_w = MIN(MAX_IMG_WIDTH, ALIGN_UP((uint32_t)(dwe_info->src_h / 2 * VS_PI) , 16));
            dwe_info->dst_h = dwe_info->src_h;
        } else if ((params->split_vertical_line_up > blocksize &&
                      params->split_vertical_line_up < dwe_info->src_w) &&
                     (params->split_vertical_line_down > blocksize &&
                     params->split_vertical_line_down < dwe_info->src_w) &&
                     (params->split_horizon_line > blocksize &&
                     params->split_horizon_line < dwe_info->src_h)) {
            // four
            dwe_info->dst_w = dwe_info->src_w;
            dwe_info->dst_h = dwe_info->src_h;
        } else if ((params->split_vertical_line_up > blocksize &&
                  params->split_vertical_line_up < dwe_info->src_w) &&
                  (params->split_vertical_line_down > blocksize &&
                   params->split_vertical_line_down < dwe_info->src_w) &&
                   params->split_vertical_line_up == params->split_vertical_line_down &&
                   params->split_horizon_line > dwe_info->src_h) {
            // left right
            dwe_info->dst_w = MIN(MAX_IMG_WIDTH, ALIGN_UP((uint32_t)(dwe_info->src_h * VS_PI) , 16));
            dwe_info->dst_h = ALIGN_UP(dwe_info->src_h / 2 , 8);
        }

        dwe_info->map_w = (ALIGN_UP(dwe_info->dst_w, blocksize) >> blockshift) + 1;
        dwe_info->map_h = (ALIGN_UP(dwe_info->dst_h, blocksize) >> blockshift) + 1;
        dwe_info->map_w++;
        dwe_info->map_h++;
    } else {
        dwe_info->roi_x = ((dwe_info->roi_x >> blockshift) << blockshift);
        dwe_info->roi_y = ((dwe_info->roi_y >> blockshift) << blockshift);

        dwe_info->dst_w = ALIGN_UP(((dwe_info->src_w - dwe_info->roi_x)*params->scale_factor) >> 12, 16);
        dwe_info->dst_h = ALIGN_UP(((dwe_info->src_h - dwe_info->roi_y)*params->scale_factor) >> 12, 8);
    }

    if (params->output_res[0].width > 0) {
        dwe_info->dst_w = ALIGN_UP(params->output_res[0].width, 16);
        dwe_info->dst_h = ALIGN_UP(params->output_res[0].height, 8);
    }
/*
    if(params->rotation){
        std::swap(dwe_info->dst_w, dwe_info->dst_h);
    }
*/
   dwe_info->src_stride = ALIGN_UP(dwe_info->src_w*(dwe_info->in_yuvbit+1), DEWARP_BUFFER_ALIGNMENT);
   dwe_info->dst_stride = ALIGN_UP(dwe_info->dst_w*(dwe_info->out_yuvbit+1), DEWARP_BUFFER_ALIGNMENT);

    if (params->input_res[0].format == MEDIA_PIX_FMT_YUV422I) {
        dwe_info->src_stride *= 2;
    }
    if (params->output_res[0].format == MEDIA_PIX_FMT_YUV422I) {
        dwe_info->dst_stride *= 2;
    }

    if (params->rotation){
        dwe_info->split_line = true;
    } else {
        dwe_info->split_line = params->dewarp_type == DEWARP_MODEL_SPLIT_SCREEN;
    }
    dwe_info->scale_factor = (uint32_t)((512*1024.0f / params->scale_factor)) & 0xffff;
    dwe_info->in_format = params->input_res[0].format;
    dwe_info->out_format = params->output_res[0].format;

    dwe_info->hand_shake = 0;
    dwe_info->boundary_y = params->boundary_pixel.Y;
    dwe_info->boundary_u = params->boundary_pixel.U;
    dwe_info->boundary_v = params->boundary_pixel.V;
    dwe_info->src_auto_shadow = dwe_info->dst_auto_shadow = 0;
    dwe_info->split_h = params->split_horizon_line;
    dwe_info->split_v1 = params->split_vertical_line_up;
    dwe_info->split_v2 = params->split_vertical_line_down;
    if (dwe_info->out_format == MEDIA_PIX_FMT_YUV420SP)
        dwe_info->dst_size_uv = ALIGN_UP(dwe_info->dst_stride*dwe_info->dst_h/2, DEWARP_BUFFER_ALIGNMENT);
    else
        dwe_info->dst_size_uv = ALIGN_UP(dwe_info->dst_stride*dwe_info->dst_h, DEWARP_BUFFER_ALIGNMENT);

    for (int i = 1; i < 4; i++) {
        if (!params->output_res[i].enable) continue;
        vse_info->out_size[i-1].width = params->output_res[i].width;
        vse_info->out_size[i-1].height = params->output_res[i].height;
        vse_info->resize_enable[i-1] = params->output_res[i].enable;
        vse_info->mi_settings[i-1].enable = vse_info->resize_enable[i-1];
    }

    vse_info->src_w = params->input_res[1].width;
    vse_info->src_h = params->input_res[1].height;
    vse_info->in_format = params->input_res[1].format;
    vse_info->in_yuvbit = params->input_res[1].yuvbit;
    vse_info->input_select = params->vse_inputSelect;
    memcpy(vse_info->crop_size, params->vse_cropSize, sizeof(vse_info->crop_size));
    memcpy(vse_info->format_conv, params->vse_format_conv, sizeof(vse_info->format_conv));
    memcpy(vse_info->mi_settings, params->mi_settings, sizeof(vse_info->mi_settings));
}

static void createBypassMap(unsigned int* warpMap, int widthMap, int heightMap, int widthImg, int heightImg)
{
    int y = 0;
    for (int i = 0; i < heightMap; i++, y += BLOCK_SIZE) {
       if ((heightMap - 1) == i)
           y = heightImg - 1;
       int x = 0;
       for (int j = 0; j < widthMap; j++, x += BLOCK_SIZE) {
           if ((widthMap - 1) == j)
              x = widthImg - 1;
           int dx = (x * 16) & 0xffff;
           int dy = (y * 16) & 0xffff;
           warpMap[i*widthMap + j] = (dy << 16) | dx;
       }
    }   
}

void set_dmap(
    struct dewarp_distortion_map* dmap,
    const struct dw200_parameters* params,
    struct k_dwe_hw_info* dwe_info,
    const char* usermap,
    unsigned int* map_buffer
) {
    if (usermap[0] != '\0') {
        pr_info("use usermap");
        int map_size = load_user_map(usermap, map_buffer);
        dmap->userMapSize = map_size;
        return;
    }

    int offsetx_p = 0;
    int offsety_p = 0;
    // double scalef_p = 1.0;
    // computePerspectiveFront(dmap->perspective_matrix, dwe_info->src_w, dwe_info->src_h, &scalef_p, &offsetx_p, &offsety_p);
    float scalefx_p = 1.0f;
    float scalefy_p = 1.0f;
    int map_bits = 16;
    if (params->bypass) {
        // create bypass map
        pr_info("use bypass map");
        createBypassMap(map_buffer, dwe_info->map_w, dwe_info->map_h,dwe_info->src_w, dwe_info->src_h);
    } else if (params->dewarp_type == DEWARP_MODEL_SPLIT_SCREEN) {
        pr_info("use polar map");
        CreateUpdateWarpPolarMap(map_buffer, dwe_info->map_w, dwe_info->map_h, map_bits, 4, dwe_info->src_w, dwe_info->src_h, dwe_info->dst_w, 
            dwe_info->dst_h, dwe_info->src_w / 2, dwe_info->src_h / 2, dwe_info->src_h / 2,
            params->split_horizon_line, params->split_vertical_line_up,
            params->split_vertical_line_down, BLOCK_SIZE, BLOCK_SIZE, 0x20, 
            params->fov.offAngleUL,params->fov.offAngleUR, params->fov.offAngleDL,params->fov.offAngleDR, params->fov.fovUL, params->fov.fovUR,
            params->fov.fovDL, params->fov.fovDR, params->fov.panoAtWin,params->fov.centerOffsetRatioUL,params->fov.centerOffsetRatioUR,
            params->fov.centerOffsetRatioDL,params->fov.centerOffsetRatioDR,params->fov.circleOffsetRatioUL,params->fov.circleOffsetRatioUR,
            params->fov.circleOffsetRatioDL,params->fov.circleOffsetRatioDR);

    } else {
        switch (params->dewarp_type) {
            case DEWARP_MODEL_LENS_DISTORTION_CORRECTION:
                pr_info("use dewarp map");
                CreateUpdateDewarpMap(map_buffer, dwe_info->map_w, dwe_info->map_h, map_bits, 4, dmap->camera_matrix, dmap->distortion_coeff,
                                        dwe_info->src_w, dwe_info->src_h, 1.0, BLOCK_SIZE, BLOCK_SIZE);
                break;
            case DEWARP_MODEL_FISHEYE_EXPAND:
                pr_info("use fisheye expand map");
                CreateUpdateFisheyeExpandMap(map_buffer, dwe_info->map_w, dwe_info->map_h, map_bits, 4, dwe_info->src_w, dwe_info->src_h, dwe_info->src_w, 
                    dwe_info->src_h, dwe_info->src_w / 2, dwe_info->src_h / 2, dwe_info->src_h / 2, BLOCK_SIZE, BLOCK_SIZE);
                break;
            case DEWARP_MODEL_FISHEYE_DEWARP:
                pr_info("use fisheye dewarp map");
                CreateUpdateFisheyeDewarpMap(map_buffer, dwe_info->map_w, dwe_info->map_h, map_bits, 4,  dmap->camera_matrix, dmap->distortion_coeff,
                    dwe_info->src_w, dwe_info->src_h, 1.0, BLOCK_SIZE, BLOCK_SIZE);
                break;
            case DEWARP_MODEL_PERSPECTIVE:
                pr_info("use perspective map");
                CreateUpdatePerspectiveMap(map_buffer, dwe_info->map_w, dwe_info->map_h, map_bits, 4, dmap->perspective_matrix, dwe_info->src_w, dwe_info->src_h,
                    BLOCK_SIZE, BLOCK_SIZE,scalefx_p, scalefy_p, offsetx_p, offsety_p);
                break;
        }
    }

    // set flip
    if(params->rotation) {
        unsigned tmp = dwe_info->map_w;
        dwe_info->map_w = dwe_info->map_h;
        dwe_info->map_h = tmp;
    }
}

static const char* format_postfix[] = {
    "yuv422sp", "yuv422i",
    "yuv420sp", "yuv444",
    "rgb888", "rgb888p",
    "raw8", "raw12"
};

void save_images(struct dw200_parameters* params, struct vbuffer* output_buffer) {
    for (unsigned i = 0; i < 4; i++) {
        if (params->output_res[i].enable && (output_buffer[i].virt_addr) != NULL && params->output_res[i].width && params->output_res[i].height) {
            char name[130] = {0};
            memset(name, 0, sizeof(name));
            snprintf(name, sizeof(name), "channel%u_%ux%u_%s.bin", i,
                params->output_res[i].width,
                params->output_res[i].height,
                format_postfix[params->output_res[i].format]
            );
            FILE* f = fopen(name, "w");
            if (f == NULL) {
                perror("open output file error");
                return;
            }
            kd_mpi_sys_mmz_flush_cache(output_buffer[i].phy_addr, output_buffer[i].virt_addr, output_buffer[i].size);
            pr_info("write channel %u to file, %p size: %lu",
                i,
                output_buffer[i].virt_addr,
                fwrite(output_buffer[i].virt_addr, 1, output_buffer[i].size, f)
            );
            fclose(f);
        }
    }
}

#define bbzero(x) bzero(&x,sizeof(x))

int main(int argc, char* argv[]) {
    pr_info("version: %s %s", __DATE__, __TIME__);
    if (argc != 2) {
        // test open
        kd_mpi_dewarp_init();
        kd_mpi_dewarp_exit();
        pr_info("Usage: %s <config JSON>", argv[0]);
        return -1;
    }

    struct dw200_parameters params;
    struct dewarp_distortion_map dmap[2];
    uint64_t golden_hash_key = 0;
    bbzero(params);
    params.mi_settings[0].enable = 0;
    params.mi_settings[1].enable = 0;
    params.mi_settings[2].enable = 0;
    char usermap[130];
    bbzero(usermap);
    char input_image_file[2][130];
    bbzero(input_image_file);

    if (parse(
        argv[1],
        input_image_file[0],
        input_image_file[1],
        usermap, dmap,
        &params, &golden_hash_key) < 0) {
            return -1;
    }
    pr_info("parse done, input0: %s, input1: %s", input_image_file[0], input_image_file[1]);
    pr_info("camera_matrix: %f %f %f %f %f %f %f %f %f",
        dmap->camera_matrix[0],
        dmap->camera_matrix[1],
        dmap->camera_matrix[2],
        dmap->camera_matrix[3],
        dmap->camera_matrix[4],
        dmap->camera_matrix[5],
        dmap->camera_matrix[6],
        dmap->camera_matrix[7],
        dmap->camera_matrix[8]);
    pr_info("distortion_coeff: %f %f %f %f %f %f %f %f",
        dmap->distortion_coeff[0],
        dmap->distortion_coeff[1],
        dmap->distortion_coeff[2],
        dmap->distortion_coeff[3],
        dmap->distortion_coeff[4],
        dmap->distortion_coeff[5],
        dmap->distortion_coeff[6],
        dmap->distortion_coeff[7]);
    struct k_dwe_hw_info dwe_info;
    struct k_vse_params vse_info;
    bzero(&dwe_info, sizeof(dwe_info));
    bzero(&vse_info, sizeof(vse_info));
    set_params(&params, &dwe_info, &vse_info);

    struct vbuffer input_buffer[2], output_buffer[4];
    bbzero(input_buffer);
    bbzero(output_buffer);
#define FOR_INPUT_BUFFER for (unsigned i = 0; i < sizeof(input_buffer) / sizeof(struct vbuffer); i++)
#define FOR_OUTPUT_BUFFER for (unsigned i = 0; i < sizeof(output_buffer) / sizeof(struct vbuffer); i++)

    // alloc buffer
    #define MMZ_NAME "anonymous"
    #define MMB_NAME "vivdw200_mmz"
#define CHECK_ERROR(x) do{pr_info("enter %s", #x);int v=x;if((v)!=0){pr_err("error %d at %s:%d",v,__FILE_NAME__,__LINE__);goto cleanup;}}while(0)

    // lut map
    struct vbuffer lut_map;
    bbzero(lut_map);
    if (params.input_res[0].enable) {
        lut_map.size = MAX_MAP_SIZE;
        CHECK_ERROR(kd_mpi_sys_mmz_alloc(
            &lut_map.phy_addr,
            &lut_map.virt_addr,
            MMB_NAME, MMZ_NAME,
            lut_map.size
        ));
        pr_info("LUT map buffer: %08X,%p", (u32)lut_map.phy_addr, lut_map.virt_addr);
    }
    set_dmap(dmap, &params, &dwe_info, usermap, lut_map.virt_addr);
    // dump dmap
    #if 0
    char name[130];
    sprintf(name, "b_dmap_%ux%u.bin", dwe_info.map_w, dwe_info.map_h);
    FILE* f = fopen(name, "w");
    if (f == NULL) {
        pr_warn("open dmap error, skip");
    } else {
        pr_info("dump dmap %ux%u", dwe_info.map_w, dwe_info.map_h);
        fwrite(lut_map.virt_addr, 4, dwe_info.map_w * dwe_info.map_h, f);
        fclose(f);
    }
    #endif
    kd_mpi_sys_mmz_flush_cache(lut_map.phy_addr, lut_map.virt_addr, lut_map.size);

    FOR_INPUT_BUFFER {
        if (params.input_res[i].enable) {
            input_buffer[i].size = resolution_size(&params.input_res[i]);
            CHECK_ERROR(kd_mpi_sys_mmz_alloc(
                &input_buffer[i].phy_addr,
                &input_buffer[i].virt_addr,
                MMB_NAME, MMZ_NAME,
                input_buffer[i].size
            ));
            pr_info("input %u buffer: %08X,%p,%u", i, (u32)input_buffer[i].phy_addr, input_buffer[i].virt_addr, input_buffer[i].size);

            // load image
            if (input_image_file[i][0] != '\0') {
                FILE* f = fopen(input_image_file[i], "r");
                if (f == NULL) {
                    perror("open image file");
                    goto cleanup;
                }
                fread(input_buffer[i].virt_addr, 1, input_buffer[i].size, f);
                fclose(f);
                kd_mpi_sys_mmz_flush_cache(input_buffer[i].phy_addr, input_buffer[i].virt_addr, input_buffer[i].size);
            }
        }
    }
    FOR_OUTPUT_BUFFER {
        if (params.output_res[i].enable) {
            output_buffer[i].size = resolution_size(&params.output_res[i]);
            CHECK_ERROR(kd_mpi_sys_mmz_alloc(
                &output_buffer[i].phy_addr,
                &output_buffer[i].virt_addr,
                MMB_NAME, MMZ_NAME,
                output_buffer[i].size
            ));
            pr_info("output%u buffer: %08X,%p,%u", i, (u32)output_buffer[i].phy_addr, output_buffer[i].virt_addr, output_buffer[i].size);
        }
    }

    // set & run
    if (kd_mpi_dewarp_init() < 0) {
        perror("open device failed");
        goto cleanup;
    }
    CHECK_ERROR(kd_mpi_dewarp_reset());

    pr_info("start %d %d %d %d, stride: %d %d",
        dwe_info.src_w, dwe_info.src_h, dwe_info.map_w, dwe_info.map_h,
        dwe_info.src_stride, dwe_info.dst_stride);
    CHECK_ERROR(kd_mpi_dewarp_dwe_disable_irq());
    CHECK_ERROR(kd_mpi_dewarp_set_map_lut_addr(lut_map.phy_addr));

    CHECK_ERROR(kd_mpi_dewarp_dwe_s_params(&dwe_info));
    uint32_t enabled_bit = 0;

    for (int i = 1; i < 4; i++) {
        if (!params.output_res[i].enable) continue;
        enabled_bit |= 1 << (i - 1);
    }
    CHECK_ERROR(kd_mpi_dewarp_vse_s_params(&vse_info));
    uint32_t ob[3] = {output_buffer[1].phy_addr, output_buffer[2].phy_addr, output_buffer[3].phy_addr};
    if (params.input_res[0].enable) {
        // processRequest
        CHECK_ERROR(kd_mpi_dewarp_update_buffer(ob));
        CHECK_ERROR(kd_mpi_dewarp_set_mi_info());
        CHECK_ERROR(kd_mpi_dewarp_set_dst_buffer_addr(output_buffer[0].phy_addr));
        CHECK_ERROR(kd_mpi_dewarp_set_map_lut_addr(lut_map.phy_addr));
        CHECK_ERROR(kd_mpi_dewarp_start_dma_read(input_buffer[0].phy_addr));
        CHECK_ERROR(kd_mpi_dewarp_enable_bus());
        CHECK_ERROR(kd_mpi_dewarp_start_dwe());

        // mainStream wait interrupt
        for (;;) {
            if (kd_mpi_dewarp_poll_irq() == 0) {
                static unsigned cnt = 0;
                cnt += 1;
                pr_warn("poll timeout");
                if (cnt > 10) {
                    pr_err("poll timeout, exit");
                    goto cleanup;
                }
                continue;
            }
            uint32_t mis = kd_mpi_dewarp_read_irq(1);
            pr_info("polled dwe irq %u", mis);
            if (mis) {
                // dwe irq
                if (mis & INT_FRAME_DONE) {
                    CHECK_ERROR(kd_mpi_dewarp_disable_irq());
                    CHECK_ERROR(kd_mpi_dewarp_disable_bus());
                    break;
                } else if (mis & INT_ERR_STATUS_MASK) {
                    pr_err("dewarp error: %d", (mis & INT_ERR_STATUS_MASK) >> INT_ERR_STATUS_SHIFT);
                } else if (mis & INT_FRAME_BUSY) {
                    pr_err("dewarp frame busy");
                }
            }
        }
        pr_info("get dwe output");
    }

    if (params.input_res[1].enable) {
        // processRequest
        if (params.vse_inputSelect == 4) {

        } else if (params.vse_inputSelect == 5) {
            CHECK_ERROR(kd_mpi_dewarp_set_dma_buffer_info(input_buffer[1].phy_addr));
        } else {
            pr_err("unsupported input select");
            goto cleanup;
        }
        CHECK_ERROR(kd_mpi_dewarp_mask_irq(0x7007));

        // mainStream wait interrupt
        for (;;) {
            if (kd_mpi_dewarp_poll_irq() == 0) {
                static unsigned cnt = 0;
                cnt += 1;
                pr_warn("poll vse timeout");
                if (cnt > 10) {
                    pr_err("poll timeout, exit");
                    goto cleanup;
                }
                continue;
            }
            uint32_t mis = kd_mpi_dewarp_read_irq(0);
            pr_info("polled vse irq %u", mis);
            static uint32_t received_irq = 0; 
            if (mis & 0x7) {
                // vse irq
                unsigned mask = 0xFFFFFFFF;
                for (unsigned i = 0; i < 3; i++) {
                    if (mis & (1<<i) && ob[i]) {
                        ob[i] = 0;
                    }
                    received_irq |= 1 << i;
                    mask &= ~(1<<i);
                }
                CHECK_ERROR(kd_mpi_dewarp_mask_irq(mask));
            }
            if ((mis & 0x2000) || (received_irq & enabled_bit) == enabled_bit) {
                received_irq = 0;
                break;
            }
        }
        pr_info("get vse output");
    }

    // save images
    sleep(1);
    save_images(&params, output_buffer);

cleanup:
    kd_mpi_dewarp_exit();
    if (params.input_res[0].enable) {
        kd_mpi_sys_mmz_free(lut_map.phy_addr, lut_map.virt_addr);
    }
    FOR_OUTPUT_BUFFER {
        if (params.output_res[i].enable) {
            kd_mpi_sys_mmz_free(output_buffer[i].phy_addr, output_buffer[i].virt_addr);
        }
    }
    FOR_INPUT_BUFFER {
        if (params.input_res[i].enable) {
            kd_mpi_sys_mmz_free(input_buffer[i].phy_addr, input_buffer[i].virt_addr);
        }
    }
    pr_info("test done");
    return 0;
}
