#include "common.h"

// Функция обработки файла (серверная часть)
int is_same_file(const char *path1, const char *path2) {
    struct stat stat1, stat2;           // здесь хранятся данные о файле
    if (stat(path1, &stat1) == 0 && stat(path2, &stat2) == 0) {
//__ino_t st_ino;		/* File serial number.	*/  по нему сравниваем, что файлы имеют одинаковый номер
//__dev_t st_dev;		/* Device.  */              по нему сравниваем, что файлы на одном устройстве
        return (stat1.st_ino == stat2.st_ino) && (stat1.st_dev == stat2.st_dev);
    }
    return 0;
}
int generate_output_filename(const char* input_file, char* output_file) {
    strcpy(output_file, input_file);
    char* dot = strrchr(output_file, '.');
    if (dot == NULL){
        strcat(output_file, ".out");
        return 0;
    }
    else if (strcmp(dot+1, "out")){
        strcpy(dot+1, "out");
        return 0;
    }
    else return 1;
}


int process_file_server(char *input, char symv) {
    int input_fd, output_fd;
    ssize_t numRead, numWritten;
    char buf;
    int count = 0;
    
    char *output_file = malloc((strlen(input) + 4) * sizeof(char));
    if (output_file == NULL) {
        printf("Error: memory allocation error!\n");
        printf("Return value %d\n", -1);
        exit(-1);
    }
    if (generate_output_filename(input, output_file)) {
        printf("Error: incorrect file extension!\n");
        free(output_file);
        printf("Return value %d\n", -1);
        exit(-1);
    }

    if (strcmp(input, output_file) == 0 || is_same_file(input, output_file)) {
        printf("Error: Input and output files are the same!\n");
        free(output_file);
        printf("Return value %d\n", -1);
        exit(-1);
    }
    char target_char = symv;
    // O_RDONLY - Открытие файла только для чтения
    
    input_fd = open(input, O_RDONLY);
    if (input_fd < 0) {
        printf("Error %d (%s) while opening input file: %s!\n", errno, strerror(errno), input);
        free(output_file);
        printf("Return value %d\n", -1);
        exit(-1);
    }

    // O_WRONLY - Открытие файла только для записи 
    // O_CREAT - создать файл если не существует
    // O_TRUNC - Усечение существующего файла до нулевой длины 
    // 0b110100100 - права доступа: Владелец: чтение+запись, Группа: чтение, Остальные: только чтение
    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0b110100100);
    if (output_fd < 0) {
        printf("Error %d (%s) while opening output file: %s!\n", errno, strerror(errno), output_file);
        close(input_fd);
        free(output_file);
        printf("Return value %d\n", -1);
        exit(-1);
    }
    
    struct flock lock;
    
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    
    if (fcntl(input_fd, F_SETLK, &lock) == -1) {
        printf("Error locking input file for reading!\n");
        close(input_fd);
        close(output_fd);
        free(output_file);
        printf("Return value %d\n", -1);
        exit(-1);
    }
    
    lock.l_type = F_WRLCK;    // Write lock
    
    if (fcntl(output_fd, F_SETLK, &lock) < 0) {
        printf("Error locking output file for writing!\n");
        lock.l_type = F_UNLCK;
        fcntl(input_fd, F_SETLK, &lock);
        close(input_fd);
        close(output_fd);
        free(output_file);
        printf("Return value %d\n", -1);
        exit(-1);
    }
    
    while ((numRead = read(input_fd, &buf, 1)) > 0) {
        if (buf == target_char) {
            buf = ' ';
            count++;
        }
        numWritten = write(output_fd, &buf, 1);
        if (numWritten != 1) {
            printf("Error writing to output file! Written: %zd, expected: 1\n", numWritten);

            lock.l_type = F_UNLCK;
            fcntl(input_fd, F_SETLK, &lock);
            fcntl(output_fd, F_SETLK, &lock);
            
            close(input_fd);
            close(output_fd);
            free(output_file);
            printf("Return value %d\n", -1);
            exit(-1);
        }
    }
    
    // Проверка ошибки чтения
    if (numRead < 0) {
        printf("Error reading from input file! Error: %s\n", strerror(errno));

        lock.l_type = F_UNLCK;
        fcntl(input_fd, F_SETLK, &lock);
        fcntl(output_fd, F_SETLK, &lock);
        
        close(input_fd);
        close(output_fd);
        free(output_file);
        printf("Return value %d\n", -1);
        exit(-1);
    }
    
    lock.l_type = F_UNLCK;
    fcntl(input_fd, F_SETLK, &lock);
    fcntl(output_fd, F_SETLK, &lock);
    
    close(input_fd);
    close(output_fd);
    
    printf("Successfully processed file:\n");
    printf("Input: %s\n", input);
    printf("Output: %s\n", output_file);
    printf("Replaced %d occurrences of '%c' with spaces\n", count, target_char);
    free(output_file);
    printf("Return value %d\n", count);
    
    return count;
}

void cleanup_fifo() {
    unlink(SERVER_FIFO);
}

int main() {
    printf("Server: Starting FIFO server...\n");
    
    atexit(cleanup_fifo);
    
    

    if (mkfifo(SERVER_FIFO, 0666) == -1) {
        if (errno != EEXIST) {
            fprintf(stderr, "Server: Error creating server FIFO: %s\n", strerror(errno));
            return FAIL;
        } else {
            printf("Server: Using existing FIFO at %s\n", SERVER_FIFO);
        }
    } else {
        printf("Server: FIFO created at %s\n", SERVER_FIFO);
    }

    int server_fd, client_fd;
    request_t request;
    int result;

    
    printf("Server: Waiting for client requests...\n");
    
    server_fd = open(SERVER_FIFO, O_RDONLY);
    if (server_fd == -1) {
        fprintf(stderr, "Server: Error opening server FIFO for reading: %s\n", strerror(errno));
        return FAIL;
    }
    
    printf("Server: Ready to receive requests\n");
    
    while (1) {
        ssize_t bytes_read = read(server_fd, &request, sizeof(request_t));
        
        if (bytes_read == sizeof(request_t)) {
            printf("\nServer: Received request from client PID %d\n", request.client_pid);
            printf("Server: File: %s, Symbol: '%c'\n", request.input_file, request.symbol);
            
            result = process_file_server(request.input_file, request.symbol);
            
            char client_fifo_name[MAX_FILENAME];
            snprintf(client_fifo_name, sizeof(client_fifo_name), CLIENT_FIFO_TEMPLATE, request.client_pid);
            
            client_fd = open(client_fifo_name, O_WRONLY);
            if (client_fd != -1) {
                write(client_fd, &result, sizeof(int));
                close(client_fd);
                printf("Server: Response sent to client %d\n", request.client_pid);
            } else {
                fprintf(stderr, "Server: Error opening client FIFO %s: %s\n", client_fifo_name, strerror(errno));
            }
        } else if (bytes_read == 0) {
            printf("Server: No more requests, shutting down...\n");
            break;
        } else {
            fprintf(stderr, "Server: Error reading request: %s\n", strerror(errno));
            break;
        }
    }
    
    close(server_fd);
    unlink(SERVER_FIFO);
    printf("Server: The program has been completed\n");
    
    return 0;
}