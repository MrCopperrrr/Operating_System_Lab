/*
gcc -o fork_join fork_join.c -lpthread
./fork_join
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    for(int i = 0; i < 5; i++) {
        pid_t pid = fork();
        if(pid == 0) {  // Tiến trình con
            usleep(300000 * i);  
            printf("Child %d is running (PID: %d)\n", i, getpid());
            return 0;
        }
    }
    // Join phase
    for(int i = 0; i < 5; i++) {
        wait(NULL);
    }
    printf("All children have finished\n");
    return 0;
}
