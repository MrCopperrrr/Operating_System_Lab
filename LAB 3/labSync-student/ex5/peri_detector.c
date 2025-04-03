/*
gcc -o peri_detector peri_detector.c -lpthread
./peri_detector
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int finished = 0; // Biến đánh dấu chương trình đã kết thúc

// safety check
int is_safe() {
    int found = rand() % 5; 
    if (!found) {
        return -1; 
    }
    return 0; 
}

// Hàm kiểm tra định kỳ
void *periodic_detector(void *arg) {
    while (1) {
        sleep(5); // Kiểm tra mỗi 5 giây

        pthread_mutex_lock(&lock);
        if (is_safe() == -1) {
            printf("[!] Lỗi phát hiện! Đang phục hồi...\n");

            // Hành động khôi phục
            printf("[+] Thực hiện khôi phục hệ thống!\n");

            if (finished) {
                pthread_mutex_unlock(&lock);
                break; // Thoát nếu chương trình kết thúc
            }
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    srand(time(NULL)); // Khởi tạo random
    pthread_t detector_thread;

    // Tạo thread dò lỗi định kỳ
    pthread_create(&detector_thread, NULL, periodic_detector, NULL);

    // Giả lập chạy 30 giây, sau đó kết thúc
    sleep(30);
    finished = 1;

    // Chờ thread dò lỗi kết thúc
    pthread_join(detector_thread, NULL);

    printf("Hệ thống đã tắt an toàn.\n");
    return 0;
}
