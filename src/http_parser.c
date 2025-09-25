#define _GNU_SOURCE
#include "http_parser.h"

http_parser_t* parser_create(void){
	http_parser_t* parser = malloc(sizeof(http_parser_t));
	if(!parser) return NULL;

	parser->buffer = malloc(INITIAL_BUFFER_SIZE);
	if(!parser->buffer){
		free(parser);
		return NULL;
	}

	parser->buffer_size = INITIAL_BUFFER_SIZE;
	parser->data_len = 0;
	parser->parse_pos = 0;

	parser->state = PARSE_REQUEST_LINE;
	parser->error_code = 0;
	parser->error_msg[0] = '\0';

	memset(&parser->request, 0, sizeof(http_request_t));
	parser->request.valid = 0;

	return parser;
}

void parser_destroy(http_parser_t* parser){
	if(parser){
		if(parser->buffer) free(parser->buffer);
		if(parser->request.body) free(parser->request.body);
		free(parser);
	}
}

void parser_reset(http_parser_t* parser){
	parser->state = PARSE_REQUEST_LINE;
	parser->data_len = 0;
	parser->parse_pos = 0;
	parser->error_code = 0;
	parser->error_msg[0] = '\0';

	if(parser->request.body){
		free(parser->request.body);
		parser->request.body = NULL;
	}
	memset(&parser->request, 0, sizeof(http_request_t));
}

int parser_feed(http_parser_t* parser, const char* data, size_t len){
	if(parser->data_len+len > parser->buffer_size){
		if(parser->data_len+len > MAX_REQUEST_SIZE){
			snprintf(parser->error_msg, sizeof(parser->error_msg), "Request too large");
			parser->state = PARSE_ERROR;
			return PARSE_ERROR;
		}

		size_t new_sz = parser->buffer_size;
		while(new_sz < parser->data_len+len) new_sz *= 2;

		char* new_buffer = realloc(parser->buffer, new_sz);
		if (!new_buffer) {
            snprintf(parser->error_msg, sizeof(parser->error_msg), "Memory allocation failed");
            parser->state = PARSE_ERROR;
            return PARSE_ERROR;
        }

		parser->buffer = new_buffer;
		parser->buffer_size = new_sz;
	}

	memcpy(parser->buffer+parser->data_len, data, len);
	parser->data_len += len;

	while(parser->state != PARSE_COMPLETE && parser->state != PARSE_ERROR){
		int result = process_current_state(parser);
		if (result == PARSE_NEED_MORE_DATA) {
            return PARSE_NEED_MORE_DATA;
        } else if (result == PARSE_ERROR) {
            return PARSE_ERROR;
        }
	}

	return (parser->state == PARSE_COMPLETE) ? PARSE_SUCCESS : PARSE_NEED_MORE_DATA;
}

int process_current_state(http_parser_t* parser){
	char line[2048];
	int line_result;

	switch(parser->state){
		case PARSE_REQUEST_LINE:
			line_result = extract_line(parser, line, sizeof(line));
			if (line_result == PARSE_NEED_MORE_DATA) return PARSE_NEED_MORE_DATA;
            if (line_result == PARSE_ERROR) return PARSE_ERROR;

			if(parse_request_line(line, &parser->request) < 0){
				snprintf(parser->error_msg, sizeof(parser->error_msg), "Invalid request line");
				parser->state = PARSE_ERROR;
				return PARSE_ERROR;
			}

			parser->state = PARSE_HEADERS;
			break;
		
		case PARSE_HEADERS:
			line_result = extract_line(parser, line, sizeof(line));
			if(line_result == PARSE_NEED_MORE_DATA) return PARSE_NEED_MORE_DATA;
			if(line_result == PARSE_ERROR) return PARSE_ERROR;

			if(strlen(line) == 0) {
				parser->request.valid = 1;
				parser->state = PARSE_COMPLETE;
				return PARSE_SUCCESS;
			}

			if(parse_header_line(line, &parser->request) < 0){
				snprintf(parser->error_msg, sizeof(parser->error_msg), "Invalid header line");
				parser->state = PARSE_ERROR;
				return PARSE_ERROR;
			}

			break;
		
		default:
			parser->state = PARSE_ERROR;
			return PARSE_ERROR;
	}

	return PARSE_SUCCESS;
}

int find_line_end(const char* buffer, size_t start, size_t len){
	for(size_t i = start ; i < len-1 ; i++){
		if(buffer[i] == '\r' && buffer[i+1] == '\n') return i;
	}

	return -1;
}

int extract_line(http_parser_t* parser, char* line, size_t max_len){
	int line_end = find_line_end(parser->buffer, parser->parse_pos, parser->data_len);
	if(line_end == -1) return PARSE_NEED_MORE_DATA;
	
	size_t line_len = line_end-parser->parse_pos;
	if(line_len >= max_len){
		snprintf(parser->error_msg, sizeof(parser->error_msg), "Line too long");
        parser->state = PARSE_ERROR;
        return PARSE_ERROR;
	}

	memcpy(line, parser->buffer+parser->parse_pos, line_len);
	line[line_len] = '\0';
	parser->parse_pos = line_end+2;

	return PARSE_SUCCESS;
}

int parse_request_line(const char* line, http_request_t* request){
	char* request_line = strdup(line);
	char* method = strtok(request_line, " ");
	char* path = strtok(NULL, " ");
	char* version = strtok(NULL, " ");

	if(!method || !path || !version){
		free(request_line);
		return -1;
	}

	memcpy(request->method, method, sizeof(request->method)-1);
	request->method[sizeof(request->method)-1] = '\0';

	memcpy(request->path, path, sizeof(request->path)-1);
	request->path[sizeof(request->path)-1] = '\0';

	memcpy(request->version, version, sizeof(request->version)-1);
	request->version[sizeof(request->version)-1] = '\0';

	free(request_line);
	return 0;
}

int parse_header_line(const char* line, http_request_t* request){
	if(request->header_count >= MAX_HEADERS) return -1;

	char* colon = strchr(line, ':');
	if(!colon) return -1;

	size_t name_len = colon - line;
    if (name_len >= MAX_HEADER_NAME_LEN) return -1;
	memcpy(request->headers[request->header_count].name, line, name_len);
    request->headers[request->header_count].name[name_len] = '\0';

    char *value_start = colon + 1;
    while (*value_start == ' ') value_start++;
    
    strncpy(request->headers[request->header_count].value, value_start, MAX_HEADER_VALUE_LEN - 1);
	request->headers[request->header_count].value[MAX_HEADER_VALUE_LEN - 1] = '\0';
    request->header_count++;

    return 0;
}