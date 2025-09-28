#include "common.h"
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    printf("Lab 4 - FIFO Client (Multiple Files)\n");
    
    if (argc < 3 || !((argc - 1) % 2 == 0)) {
        fprintf(stderr, "Error: Insufficient number of arguments\n");
        fprintf(stderr, "Using: %s input1.txt s1 input2.txt s2 ...\n", argv[0]);
        fprintf(stderr, "Example: %s file1.txt a file2.txt b file3.txt c\n", argv[0]);
        return FAIL;
    }
    
    int file_count = (argc - 1) / 2;
    pid_t *pids = malloc(file_count * sizeof(pid_t));
    int *results = malloc(file_count * sizeof(int));
    
    if (pids == NULL || results == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        return FAIL;
    }
    
    printf("Starting processing of %d files using FIFO...\n", file_count);
    
    // Запуск клиентских процессов для каждого файла
    for (int i = 0; i < file_count; i++) {
        char* input_file = argv[i * 2 + 1];
        char* symbol = argv[i * 2 + 2];
        int status;

        pid_t pid = fork();
        
        if (pid < 0) {
            fprintf(stderr, "Error forking process for file %s\n", input_file);
            results[i] = FAIL;
        } else if (pid == 0) {
            execl("./client", "client", input_file, symbol, NULL);
            fprintf(stderr, "Error executing client for file %s\n", input_file);
            exit(FAIL);
        } else {
            pids[i] = pid;
            printf("Started client process %d for file: %s (symbol: '%c')\n", pid, input_file, symbol[0]);
        }
    }
    
    for (int i = 0; i < file_count; i++) {
        int status;
        pid_t pid = waitpid(pids[i], &status, 0);
        
        if (pid < 0) {
            printf("Error waiting for process %d: %s\n", pids[i], strerror(errno));
            results[i] = FAIL;
        } else if (WIFEXITED(status)) {
            results[i] = WEXITSTATUS(status);
            printf("Client process %d exited with code: %d\n", pid, results[i]);
        } else {
            printf("Client process %d terminated abnormally\n", pid);
            results[i] = FAIL;
        }
    }
    
    printf("\n=== Final Processing Results ===\n");
    int total_replacements = 0;
    int successful_files = 0;
    
    for (int i = 0; i < file_count; i++) {
        printf("File %d: %s: ", i + 1, argv[i * 2 + 1]);
        
        if (results[i] >= 0) {
            printf("SUCCESS (%d replacements)\n", results[i]);
            total_replacements += results[i];
            successful_files++;
        } else {
            printf("FAILED\n");
        }
    }
    
    printf("\nSummary:\n");
    printf("- Successfully processed: %d/%d files\n", successful_files, file_count);
    printf("- Total replacements made: %d\n", total_replacements);
    
    free(pids);
    free(results);
    
    printf("\nAll files processed using FIFO!\n");
    return 0;
}