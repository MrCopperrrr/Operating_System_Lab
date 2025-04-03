/*
gcc -o prod_cons prod_cons.c -lpthread
./prod_cons
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define MAX_ITEMS 20

int buffer[BUFFER_SIZE];
int count = 0;  // Số phần tử hiện tại trong buffer

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;

// Hàm Writer (Producer) - Ghi dữ liệu vào buffer
void *writer(void *arg) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        usleep(rand() % 500000); 
        
        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&cond_full, &mutex);
        }

        // Ghi vào buffer
        buffer[count] = i;
        printf("Writer: Wrote %d to buffer[%d]\n", i, count);
        count++;

        // Báo hiệu Reader rằng buffer không rỗng
        pthread_cond_signal(&cond_empty);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Hàm Reader (Consumer) - Đọc dữ liệu từ buffer
void *reader(void *arg) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        usleep(rand() % 700000);  

        pthread_mutex_lock(&mutex);
        while (count == 0) {
            pthread_cond_wait(&cond_empty, &mutex);
        }

        // Lấy dữ liệu từ buffer
        count--;
        printf("Reader: Read %d from buffer[%d]\n", buffer[count], count);

        // Báo hiệu Writer rằng buffer chưa đầy
        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    srand(time(NULL)); 
    pthread_t writer_thread, reader_thread;

    // Tạo các thread Writer và Reader
    pthread_create(&writer_thread, NULL, writer, NULL);
    pthread_create(&reader_thread, NULL, reader, NULL);

    // Chờ các thread hoàn thành
    pthread_join(writer_thread, NULL);
    pthread_join(reader_thread, NULL);

    return 0;
}
