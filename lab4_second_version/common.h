#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FAIL -1
#define BUFFER_SIZE 1024
#define MAX_FILENAME 256
#define SERVER_FIFO "/tmp/lab4_server_fifo"
#define CLIENT_FIFO_TEMPLATE "/tmp/lab4_client_%d_fifo"

typedef struct {
    pid_t client_pid;
    char input_file[MAX_FILENAME];
    char symbol;
} request_t;

#endif