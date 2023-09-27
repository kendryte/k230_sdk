#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "posixtest.h"

int main(void)
{
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    struct dirent *entry = (struct dirent *)malloc(sizeof(struct dirent));
    struct dirent **result = (struct dirent **)malloc(sizeof(struct dirent));
    unsigned int cnt = 0;
    unsigned int ret = 0;

    pDir = opendir(".");
    if (NULL == pDir)
    {
        printf("opendir Fail\n");
        return PTS_FAIL;
    }

    ret = readdir_r(pDir, entry, result);
    if (ret)
    {
        free(entry);
        free(result);
        printf("Failed to read dir or don`t have child directory\n");
        return PTS_FAIL;
    }
    printf("name	:[%s]	\n", entry->d_name);
    printf("name	:[%s]	\n", result[0]->d_name);
    if (strlen(entry->d_name) == 0 || strlen(result[0]->d_name) == 0)
    {
        free(entry);
        free(result);
        printf("Failed to read dir or don`t have child directory\n");
        return PTS_FAIL;
    }
    free(entry);
    free(result);
    printf("{Test PASSED}\n");
    return PTS_PASS;
}
