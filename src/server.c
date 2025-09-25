#include "server.h"

void handle_client(int client_socket) {
    http_parser_t* parser = parser_create();
	char buffer[MAX_BUFFER_SIZE];

	while(1){
		size_t bytes = recv(client_socket, buffer, sizeof(buffer), 0);
		if(bytes <= 0) break;

		int result = parser_feed(parser, buffer, bytes);
		if(result == PARSE_SUCCESS){
			printf("Parsed: %s %s %s\n", parser->request.method, parser->request.path, parser->request.version);
			break;
		}else if(result == PARSE_ERROR){
			printf("Parse error: %s\n", parser->error_msg);
            break;
		}
	}

	parser_destroy(parser);
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
        printf("New connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

		handle_client(client_socket);

		close(client_socket);
		printf("Connection closed for %s:%d\n", client_ip, ntohs(client_addr.sin_port));
	}

	close(server_socket);
	return 0;
}