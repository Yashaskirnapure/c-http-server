#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "http_parser.h"

int start_server(int port);
void handle_client(int client_socket);
int parse_request_line(const char* buffer, http_request_t* request);
void send_response(int client_socket, const char* status, const char* content_type);
void handle_static_file(int client_socket, const char* path);

#endif