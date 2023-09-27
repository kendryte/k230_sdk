#include "lv_record_files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct tag_record_file_info
{
    char filename[256];
    char filepath[256];
    char filepathname[256];
    float  filesize;
}record_file_info;

#define MAX_FILES_COUNT  100
static record_file_info g_ary_files[MAX_FILES_COUNT];
static int g_files_count = 0;

static int _file_size(char* filename)
{
  struct stat statbuf;
  stat(filename,&statbuf);
  int size=statbuf.st_size;

  return size;
}

static int readFileList(char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    if ((dir = opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        return -1;
    }

    int nIndex = 0;
    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) /// current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8) /// file
        {
            if (g_files_count >= MAX_FILES_COUNT)
            {
                return -1;
            }
            char sfilepathtmp[256];
            sprintf(sfilepathtmp,"%s/%s",basePath, ptr->d_name);
            char *ext=strrchr(sfilepathtmp,'.');
            if (0 == strcmp(ext,".mp4"))
            {
                nIndex = g_files_count++;
                sprintf(g_ary_files[nIndex].filename,"%s", ptr->d_name);
                sprintf(g_ary_files[nIndex].filepath, "%s",basePath);
                sprintf(g_ary_files[nIndex].filepathname,"%s/%s",basePath, ptr->d_name);
                g_ary_files[nIndex].filesize = _file_size(g_ary_files[nIndex].filepathname)/1024/1024.0;
            }

        }
        else if (ptr->d_type == 10) /// link file
            ;
        else if (ptr->d_type == 4) /// dir
        {
            memset(base, '\0', sizeof(base));
            strcpy(base, basePath);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            readFileList(base);
        }
    }
    closedir(dir);

    if (g_files_count <= 0)
    {
        return -1;
    }

    return 0;
}

int _lv_record_files_init(char *basePath)
{
    g_files_count = 0;
    return readFileList(basePath);
}

const char *_lv_record_files_get_name(uint32_t track_id)
{
    if (track_id >= g_files_count)
        return NULL;
    return g_ary_files[track_id].filename;
}

const char *_lv_record_files_get_path(uint32_t track_id)
{
    if (track_id >= g_files_count)
        return NULL;
    return g_ary_files[track_id].filepath;
}

const char * _lv_record_files_get_filepathname(uint32_t track_id)
{
    if (track_id >= g_files_count)
        return NULL;
    return g_ary_files[track_id].filepathname;
}

float _lv_record_files_get_size(uint32_t track_id)
{
    if (track_id >= g_files_count)
        return 0;
    return g_ary_files[track_id].filesize;
}
