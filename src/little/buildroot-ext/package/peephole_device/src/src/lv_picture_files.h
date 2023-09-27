#ifndef LV_PICTURE_FILES_H
#define LV_PICTURE_FILES_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

int _lv_picture_files_init(char *basePath);
const char * _lv_picture_files_get_name(uint32_t track_id);
const char * _lv_picture_files_get_path(uint32_t track_id);
const char * _lv_picture_files_get_filepathname(uint32_t track_id);
float _lv_picture_files_get_size(uint32_t track_id);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif