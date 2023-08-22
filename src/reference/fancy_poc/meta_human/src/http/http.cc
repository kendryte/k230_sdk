#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h> 
#include "http.h"

#define BUFFER_SIZE 2048
#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/octet-stream\r\nContent-Length: %ld\r\n\r\n%s"
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"
 
 
static int http_tcpclient_create(const char *host, int port)
{
    struct hostent *he;
    struct sockaddr_in server_addr; 
    int socket_fd;
 
    if((he = gethostbyname(host))==NULL) {
        return -1;
    }
 
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(port);
   server_addr.sin_addr = *((struct in_addr *)he->h_addr);
 
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0))==-1) {
        return -1;
    }
 
    if(connect(socket_fd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1) {
        return -1;
    }
 
    return socket_fd;
}
  
static int http_parse_url(const char *url, char *host, char *file, int *port)
{
    char *ptr1,*ptr2;
    int len = 0;
    if(!url || !host || !file || !port) {
        return -1;
    }
 
    ptr1 = (char *)url;
 
    if(!strncmp(ptr1,"http://",strlen("http://"))) {
        ptr1 += strlen("http://");
    }
    else {
        return -1;
    }
 
    ptr2 = strchr(ptr1,'/');
    if(ptr2) {
        len = strlen(ptr1) - strlen(ptr2);
        memcpy(host,ptr1,len);
        host[len] = '\0';
        if(*(ptr2 + 1)) {
            memcpy(file,ptr2 + 1,strlen(ptr2) - 1 );
            file[strlen(ptr2) - 1] = '\0';
        }
    }
    else {
        memcpy(host,ptr1,strlen(ptr1));
        host[strlen(ptr1)] = '\0';
    }
    //get host and ip
    ptr1 = strchr(host,':');
    if(ptr1) {
        *ptr1++ = '\0';
        *port = atoi(ptr1);
    }
    else {
        *port = HTTP_DEFAULT_PORT;
    }
 
    return 0;
}
 
static int http_tcpclient_recv(int socket,char *lpbuff)
{
    int recvnum = 0;
 
    recvnum = recv(socket, lpbuff, BUFFER_SIZE*4, 0);
 
    return recvnum;
}
 
static int http_tcpclient_send(int socket,char *buff, int size)
{
    int sent = 0,tmpres = 0;
 
    while(sent < size) {
        tmpres = send(socket,buff+sent,size-sent,0);
        if(tmpres == -1) {
            return -1;
        }
        sent += tmpres;
    }
    return sent;
}
 
static char *http_parse_result(const char*lpbuf)
{
    char *ptmp = NULL; 
    char *response = NULL;
    ptmp = (char*)strstr(lpbuf,"HTTP/1.1");
    if(!ptmp) {
        printf("http/1.1 not faind\n");
        return NULL;
    }
    // if(atoi(ptmp + 9)!=200) {
    //     printf("result:\n%s\n",lpbuf);
    //     return NULL;
    // }
 
    ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
    if(!ptmp) {
        printf("ptmp is NULL\n");
        return NULL;
    }
    response = (char *)malloc(strlen(ptmp)+1);
    if(!response) {
        printf("malloc failed \n");
        return NULL;
    }
    strcpy(response,ptmp+4);
    return response;
}

static void http_tcpclient_close(int socket)
{
    close(socket);
}

char * http_post(const char *url,const char *post_str)
{
    char post[BUFFER_SIZE] = {'\0'};
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE*4] = {'\0'};
    char *ptmp;
    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;
    int len = 0;
	char *response = NULL;
 
    if(!url || !post_str) {
        printf("      failed!\n");
        return NULL;
    }
 
    if(http_parse_url(url, host_addr, file, &port)) {
        printf("http_parse_url failed!\n");
        return NULL;
    }
    printf("host_addr : %s\tfile:%s\t,%d\n",host_addr,file,port);
 
    socket_fd = http_tcpclient_create(host_addr,port);
    if(socket_fd < 0) {
        printf("http_tcpclient_create failed\n");
        return NULL;
    }
     
    sprintf(lpbuf, HTTP_POST, file, host_addr, port, strlen(post_str), post_str);
    //printf("send %s",lpbuf);
 
    if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf)) < 0) {
        printf("http_tcpclient_send failed..\n");
        return NULL;
    }

 #if 0
    /*it's time to recv from server*/
    if(http_tcpclient_recv(socket_fd,lpbuf) <= 0){
        printf("http_tcpclient_recv failed\n");
        return NULL;
    }
 #endif

    http_tcpclient_close(socket_fd);
 
    return http_parse_result(lpbuf);
}

#ifdef      DEBUG 
//- ./a.out http://10.100.201.105:8080 hello.world
int main(int argc, char *argv[])
{
    http_post(argv[1], argv[2]);

    return 0;
}
#endif