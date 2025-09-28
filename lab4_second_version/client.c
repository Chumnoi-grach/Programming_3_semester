#include "common.h"

int main(int argc, char* argv[]) {
    pid_t client_pid = getpid();
    printf("Client %d: Starting...\n", client_pid);
    
    if (argc != 3) {
        fprintf(stderr, "Client %d: Error: Invalid number of arguments\n", client_pid);
        fprintf(stderr, "Client %d: Using: %s input_file symbol\n", client_pid, argv[0]);
        fprintf(stderr, "Client %d: Example: %s file.txt a\n", client_pid, argv[0]);
        return FAIL;
    }
    
    char* input_file = argv[1];
    char* symbol_str = argv[2];
    
    if (strlen(symbol_str) != 1) {
        fprintf(stderr, "Client %d: Error: Symbol must be a single character\n", client_pid);
        return FAIL;
    }
    
    char symbol = symbol_str[0];
    
    
    printf("Client: PID %d, File: %s, Symbol: '%c'\n", client_pid, input_file, symbol);
    
    if (access(input_file, F_OK) == -1) {
        fprintf(stderr, "Client %d: Error: Input file %s does not exist\n", client_pid, input_file);
        return FAIL;
    }
    
    char client_fifo_name[MAX_FILENAME];
    snprintf(client_fifo_name, sizeof(client_fifo_name), CLIENT_FIFO_TEMPLATE, client_pid);
    
    if (mkfifo(client_fifo_name, 0666) == -1 && errno != EEXIST) {
        fprintf(stderr, "Client %d: Error creating client FIFO: %s\n", client_pid, strerror(errno));
        return FAIL;
    }
    
    request_t request;
    request.client_pid = client_pid;
    strncpy(request.input_file, input_file, MAX_FILENAME - 1);
    request.input_file[MAX_FILENAME - 1] = '\0';
    request.symbol = symbol;
    
    int server_fd = open(SERVER_FIFO, O_WRONLY);
    if (server_fd == -1) {
        fprintf(stderr, "Client %d: Error opening server FIFO: %s\n", client_pid, strerror(errno));
        unlink(client_fifo_name);
        return FAIL;
    }
    
    printf("Client %d:: Sending request to server...\n", client_pid);
    write(server_fd, &request, sizeof(request_t));
    close(server_fd);
    
    int client_fd = open(client_fifo_name, O_RDONLY);
    if (client_fd == -1) {
        fprintf(stderr, "Client %d: Error opening client FIFO for reading: %s\n", client_pid, strerror(errno));
        unlink(client_fifo_name);
        return FAIL;
    }
    
    int result;
    ssize_t bytes_read = read(client_fd, &result, sizeof(int));
    close(client_fd);
    
    unlink(client_fifo_name);
    
    if (bytes_read == sizeof(int)) {
        if (result >= 0) {
            printf("Client %d: SUCCESS - %d replacements made\n", client_pid, result);
            return result;
        } else {
            printf("Client %d: FAILED - Error processing file\n", client_pid);
            return FAIL;
        }
    } else {
        fprintf(stderr, "Client %d:: Error receiving response from server\n", client_pid);
        return FAIL;
    }
}