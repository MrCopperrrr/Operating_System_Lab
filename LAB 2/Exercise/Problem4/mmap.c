#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define FILE_SIZE 1024  // Shared memory size

int main() {
    char *mapped_mem;

    // Create an anonymous shared memory mapping
    mapped_mem = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Create child process
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {  // Child process (reader)
        while (mapped_mem[0] == '\0');  // Wait for parent to write
        printf("Child process reads from shared memory: %s\n", mapped_mem);
        munmap(mapped_mem, FILE_SIZE);
    } else {  // Parent process (writer)
        char input[FILE_SIZE];
        
        printf("Enter content to write to shared memory: ");
        fflush(stdout);  // Ensure prompt appears before input
        
        fgets(input, FILE_SIZE, stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character

        strcpy(mapped_mem, input);
        printf("Parent process wrote: %s\n", mapped_mem);

        wait(NULL);  // Wait for child process
        munmap(mapped_mem, FILE_SIZE);
    }

    return 0;
}
