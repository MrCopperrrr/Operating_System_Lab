/*
gcc -o rsc_manager rsc_manager.c -lpthread
./rsc_manager
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_RESOURCES 5  
#define NUM_PROCESSES 10 

typedef struct {
    int id;   
    int requested_resources; 
    void (*callback)(int);   
} process_request_t;

int available_resources = NUM_RESOURCES;
pthread_mutex_t resource_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t resource_cond = PTHREAD_COND_INITIALIZER;

// Hàm callback khi tài nguyên sẵn sàng
void resource_callback(int process_id) {
    printf("[+] Process %d: Được cấp phát tài nguyên!\n", process_id);
    sleep(1); 
}

// Hàm quản lý cấp phát tài nguyên
void *resource_manager(void *arg) {
    process_request_t *request = (process_request_t *)arg;

    pthread_mutex_lock(&resource_lock);
    
    // Nếu không đủ tài nguyên, tiến trình sẽ chờ
    while (request->requested_resources > available_resources) {
        printf("[!] Process %d: Đang chờ tài nguyên...\n", request->id);
        pthread_cond_wait(&resource_cond, &resource_lock);
    }

    // Cấp phát tài nguyên
    available_resources -= request->requested_resources;
    request->callback(request->id); 

    // Giải phóng tài nguyên sau khi sử dụng
    available_resources += request->requested_resources;
    printf("[✓] Process %d: Giải phóng tài nguyên!\n", request->id);

    // Thông báo cho các tiến trình khác
    pthread_cond_broadcast(&resource_cond);
    
    pthread_mutex_unlock(&resource_lock);
    free(request);
    return NULL;
}

int main() {
    pthread_t threads[NUM_PROCESSES];

    for (int i = 0; i < NUM_PROCESSES; i++) {
        process_request_t *req = malloc(sizeof(process_request_t));
        req->id = i;
        req->requested_resources = (rand() % 3) + 1; 
        req->callback = resource_callback;

        pthread_create(&threads[i], NULL, resource_manager, (void *)req);
        usleep(50000); 
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Tất cả tiến trình đã hoàn thành.\n");
    return 0;
}
