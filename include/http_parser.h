#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <sys/types.h>
#include "common.h"

#define MAX_HEADERS 50
#define MAX_HEADER_NAME_LEN 256
#define MAX_HEADER_VALUE_LEN 1024
#define INITIAL_BUFFER_SIZE 4096
#define MAX_REQUEST_SIZE 65536

typedef enum {
    PARSE_REQUEST_LINE,
    PARSE_HEADERS,
    PARSE_BODY,
    PARSE_COMPLETE,
    PARSE_ERROR
} parser_state_t;

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

typedef struct {
	parser_state_t state;
    
    char* buffer;
    size_t buffer_size;
    size_t data_len;
    size_t parse_pos;
    
    http_request_t request;
    
    int error_code;
    char error_msg[256];
} http_parser_t;

http_parser_t* parser_create(void);
void parser_destroy(http_parser_t* parser);
int parser_feed(http_parser_t* parser, const char* data, size_t len);
void parser_reset(http_parser_t* parser);

int process_current_state(http_parser_t* parser);
int find_line_end(const char* buffer, size_t start, size_t len);
int extract_line(http_parser_t* parser, char* line, size_t max_len);
int parse_request_line(const char *line, http_request_t *request);
int parse_header_line(const char *line, http_request_t *request);

#define PARSE_NEED_MORE_DATA 0
#define PARSE_SUCCESS 1
#define PARSE_ERROR -1

#endif