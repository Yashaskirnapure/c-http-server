#include "common.h"
#include "server.h"

void print_usage(const char *program_name) {
    printf("Usage: %s [port]\n", program_name);
    printf("Default port: %d\n", DEFAULT_PORT);
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number\n");
            print_usage(argv[0]);
            return 1;
        }
    }
    
    printf("Starting HTTP server on port %d...\n", port);
    printf("Serving files from: %s\n", STATIC_DIR);
    
    if (start_server(port) < 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    
    return 0;
}