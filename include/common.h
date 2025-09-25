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
#include <errno.h>

#define DEFAULT_PORT 8080
#define MAX_BUFFER_SIZE 4096
#define MAX_PATH_LENGTH 1024
#define STATIC_DIR "./static"

#define HTTP_200_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_404_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define HTTP_400_BAD_REQUEST "HTTP/1.1 400 Bad Request\r\n"
#define HTTP_500_INTERNAL_ERROR "HTTP/1.1 500 Internal Server Error\r\n"

#endif