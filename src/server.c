#include "server.h"
#include <pthread.h>

void handle_static_file(int client_socket, const char* path){
    char file_path[MAX_PATH_LENGTH];
    const char* resolved_path = path;

    if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
        resolved_path = "/index.html";
    }

    if(strstr(resolved_path, "..")){
        send_response(client_socket, HTTP_403_FORBIDDEN, "text/html");
        return;
    }

    snprintf(file_path, sizeof(file_path), "%s%s", STATIC_DIR, resolved_path);

    int fd = open(file_path, O_RDONLY);
    if(fd < 0){
        send_response(client_socket, HTTP_404_NOT_FOUND, "text/html");
        return;
    }

    struct stat st;
    if(fstat(fd, &st) < 0){
        close(fd);
        send_response(client_socket, HTTP_500_INTERNAL_ERROR, "text/html");
        return;
    }

    char header[MAX_BUFFER_SIZE];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %zu\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n",
        (size_t)st.st_size
    );
    send(client_socket, header, header_len, 0);

    char buffer[4096];
    ssize_t n;
    while((n = read(fd, buffer, sizeof(buffer))) > 0){
        send(client_socket, buffer, n, 0);
    }

    close(fd);
}

const char* get_html_file_for_status(const char* status) {
    int code;

    if (strcmp(status, HTTP_400_BAD_REQUEST) == 0) code = 400;
    else if (strcmp(status, HTTP_403_FORBIDDEN) == 0) code = 403;
    else if (strcmp(status, HTTP_404_NOT_FOUND) == 0) code = 404;
    else code = 500;

    switch (code) {	
        case 400: return "response/400.html";
        case 403: return "response/403.html";
        case 404: return "response/404.html";
        case 500:
        default:  return "response/500.html";
    }
}

void send_response(int client_socket, const char* status, const char* content_type) {
    char file_path[MAX_PATH_LENGTH];
    snprintf(file_path, sizeof(file_path), "%s/%s", STATIC_DIR, get_html_file_for_status(status));

    printf(file_path);
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        const char* body = "<h1>500 Internal Server Error</h1>";

        char header[MAX_BUFFER_SIZE];
        int header_len = snprintf(header, sizeof(header),
                                  "HTTP/1.1 500 Internal Server Error\r\n"
                                  "Content-Type: text/html\r\n"
                                  "Content-Length: %zu\r\n"
                                  "Connection: close\r\n"
                                  "\r\n",
                                  strlen(body));
        send(client_socket, header, header_len, 0);
        send(client_socket, body, strlen(body), 0);
        printf("Could not open file.");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return;
    }

    char header[MAX_BUFFER_SIZE];
    int header_len = snprintf(header, sizeof(header),
                              "%s"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status, content_type, (size_t)st.st_size);
    send(client_socket, header, header_len, 0);

    char buffer[4096];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        send(client_socket, buffer, n, 0);
    }

    close(fd);
}

void* handle_client(void* socket) {
    int client_socket = *(int*)socket;
    free(socket);

    http_parser_t* parser = parser_create();
	char buffer[MAX_BUFFER_SIZE];

	while(1){
		size_t bytes = recv(client_socket, buffer, sizeof(buffer), 0);
		if(bytes <= 0) break;

		int result = parser_feed(parser, buffer, bytes);
		if(result == PARSE_SUCCESS){
			printf("Parsed: %s %s %s\n", parser->request.method, parser->request.path, parser->request.version);
			if(strcmp(parser->request.method, "GET") == 0){
				handle_static_file(client_socket, parser->request.path);
			}else{
				send_response(client_socket, HTTP_400_BAD_REQUEST, "text/html");
			}
			break;
		}else if(result == PARSE_ERROR){
			printf("Parse error: %s\n", parser->error_msg);
            break;
		}
	}

	parser_destroy(parser);
    close(client_socket);
    printf("Connection closed for %d\n", client_socket);

    return NULL;
}

int start_server(int port){
	int server_socket;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket < 0){
		perror("Failed to create socket");
        return -1;
	}

	int opt = 1;
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		perror("setsockopt failed");
        close(server_socket);
        return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if(bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		perror("Failed to bind socket.");
		close(server_socket);
		return -1;
	}

	if (listen(server_socket, 10) < 0) {
        perror("Failed to listen");
        close(server_socket);
        return -1;
    }

	printf("Server listening on 0.0.0.0:%d\n", port);

	while(1){
		int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client_socket < 0) {
            perror("Failed to accept connection");
            continue;
        }

		char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("New connection from %s:%d on %d\n", client_ip, ntohs(client_addr.sin_port), client_socket);

        int* socket = (int*)malloc(sizeof(int));
        *socket = client_socket;

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, socket);
        pthread_detach(thread);
	}

	close(server_socket);
	return 0;
}