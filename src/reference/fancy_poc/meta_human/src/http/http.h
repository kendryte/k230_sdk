#ifndef __HTTP_H
#define __HTTP_H

//#define     DEBUG
#define     HTTP_DEFAULT_PORT       80
 
char * http_get(const char *url);
char * http_post(const char *url,const char *post_str);
 
#endif