#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#define STATIC_DIR "static"
#define DEFAULT_PORT 8080
#define MAX_BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024
#define MAX_HEADERS 50
#define MAX_HEADER_NAME_LEN 256
#define MAX_HEADER_VALUE_LEN 1024
#define INITIAL_BUFFER_SIZE 4096
#define MAX_REQUEST_SIZE 65536

typedef struct {
    char name[MAX_HEADER_NAME_LEN];
    char value[MAX_HEADER_VALUE_LEN];
} http_header_t;

typedef struct {
    char method[16];
    char path[MAX_PATH_LENGTH];
    char version[16];
    http_header_t headers[MAX_HEADERS];
    int header_count;
    char* body;
    size_t body_length;
    size_t content_length;
    int valid;
} http_request_t;

#define HTTP_200_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_403_FORBIDDEN "HTTP/1.1 403 Forbidden\r\n"
#define HTTP_404_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define HTTP_400_BAD_REQUEST "HTTP/1.1 400 Bad Request\r\n"
#define HTTP_500_INTERNAL_ERROR "HTTP/1.1 500 Internal Server Error\r\n"

#endif