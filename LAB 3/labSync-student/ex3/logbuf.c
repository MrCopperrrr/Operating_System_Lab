/*
gcc -o logbuf logbuf.c -lpthread
./logbuf
*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_LOG_LENGTH 10
#define MAX_BUFFER_SLOT 6
#define MAX_LOOPS 30

char logbuf[MAX_BUFFER_SLOT][MAX_LOG_LENGTH];
int log_count = 0; 

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t log_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t log_not_empty = PTHREAD_COND_INITIALIZER;

struct _args {
    unsigned int interval;
};

// Hàm ghi log (thread-safe)
void *wrlog(void *data) {
    char str[MAX_LOG_LENGTH];
    int id = *(int*) data;

    usleep(20);
    sprintf(str, "%d", id);

    pthread_mutex_lock(&log_mutex);
    while (log_count >= MAX_BUFFER_SLOT) {
        pthread_cond_wait(&log_not_full, &log_mutex);
    }

    // Ghi log vào buffer
    strcpy(logbuf[log_count], str);
    log_count++;
    printf("wrlog(): %d \n", id);

    // Báo hiệu rằng có log mới
    pthread_cond_signal(&log_not_empty);
    pthread_mutex_unlock(&log_mutex);

    return NULL;
}

// Hàm flush log (xóa log)
void flushlog() {
    pthread_mutex_lock(&log_mutex);
    while (log_count == 0) {
        pthread_cond_wait(&log_not_empty, &log_mutex);
    }

    printf("flushlog()\n");
    for (int i = 0; i < log_count; i++) {
        printf("Slot  %i: %s\n", i, logbuf[i]);
    }

    // Reset buffer
    log_count = 0;

    // Báo hiệu rằng buffer có thể ghi thêm log
    pthread_cond_broadcast(&log_not_full);
    pthread_mutex_unlock(&log_mutex);
}

// Timer tự động flush log
void *timer_start(void *args) {
    while (1) {
        usleep(((struct _args *) args)->interval);
        flushlog();
    }
}

int main() {
    pthread_t tid[MAX_LOOPS], lgrid;
    int id[MAX_LOOPS];
    struct _args args;
    args.interval = 500e3; // 500ms

    // Tạo thread để flush log theo thời gian
    pthread_create(&lgrid, NULL, &timer_start, (void*)&args);

    // Tạo các thread ghi log
    for (int i = 0; i < MAX_LOOPS; i++) {
        id[i] = i;
        pthread_create(&tid[i], NULL, wrlog, (void*)&id[i]);
    }

    for (int i = 0; i < MAX_LOOPS; i++) {
        pthread_join(tid[i], NULL);
    }

    sleep(5);
    return 0;
}
